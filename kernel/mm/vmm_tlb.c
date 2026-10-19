#include "velum/irqflags.h"
#include "velum/irq.h"
#include "velum/panic.h"
#include "vmm_int.h"
#include "vmm_weak.h"

#pragma weak apic_eoi
#pragma weak panic

uint32_t	vmm_cpu_total(void)
{
	uint32_t	n;

	if (!smp_cpu_count)
		return (1);
	n = smp_cpu_count();
	if (n < 1)
		return (1);
	if (n > VMM_CPUS)
		return (VMM_CPUS);
	return (n);
}

uint32_t	vmm_cpu_id(void)
{
	uint32_t	id;

	if (!smp_cpu_id)
		return (0);
	id = smp_cpu_id();
	if (id >= VMM_CPUS)
		return (0);
	return (id);
}

t_aspace	*vmm_cpu_current(void)
{
	uint32_t	id;

	id = vmm_cpu_id();
	if (id == 0)
		return (g_vmm.current);
	return (g_vmm.tlb.cur[id]);
}

void	vmm_cpu_set_current(t_aspace *as)
{
	uint32_t	id;

	id = vmm_cpu_id();
	if (id == 0)
		g_vmm.current = as;
	else
		g_vmm.tlb.cur[id] = as;
}

void	vmm_tlb_attach(t_aspace *as)
{
	if (as && !as->kernel)
		__atomic_fetch_or(&as->cpumask, 1ull << vmm_cpu_id(),
			__ATOMIC_SEQ_CST);
}

void	vmm_tlb_detach(t_aspace *as)
{
	if (as && !as->kernel)
		__atomic_fetch_and(&as->cpumask, ~(1ull << vmm_cpu_id()),
			__ATOMIC_SEQ_CST);
}

static void	flush_request(const t_tlbreq *rq)
{
	uintptr_t	va;

	if (rq->hi - rq->lo > TLB_RANGE_PAGES * PAGE_SIZE)
	{
		mmu_flush_global();
		return ;
	}
	va = align_down(rq->lo, PAGE_SIZE);
	mmu_invlpg(va);
	va += PAGE_SIZE;
	while (va < rq->hi)
	{
		mmu_invlpg(va);
		va += PAGE_SIZE;
	}
}

void	vmm_tlb_service(void)
{
	uint64_t	bit;

	bit = 1ull << vmm_cpu_id();
	if (!(__atomic_load_n(&g_vmm.tlb.pending, __ATOMIC_ACQUIRE) & bit))
		return ;
	flush_request(&g_vmm.tlb.req);
	__atomic_fetch_and(&g_vmm.tlb.pending, ~bit, __ATOMIC_RELEASE);
}

void	vmm_tlb_poll(void)
{
	if (__atomic_load_n(&g_vmm.tlb.pending, __ATOMIC_RELAXED))
		vmm_tlb_service();
}

void	vmm_tlb_ipi(t_regs *regs, void *ctx)
{
	(void)regs;
	(void)ctx;
	vmm_tlb_service();
	if (apic_eoi)
		apic_eoi();
}

static uint64_t	targets(const t_aspace *as, uint32_t self)
{
	uint64_t	all;
	uint64_t	mask;
	uint32_t	n;

	n = vmm_cpu_total();
	all = ~0ull;
	if (n < VMM_CPUS)
		all = (1ull << n) - 1;
	mask = all;
	if (!as->kernel)
		mask = __atomic_load_n(&as->cpumask, __ATOMIC_SEQ_CST) & all;
	return (mask & ~(1ull << self));
}

static void	send_to(uint64_t mask)
{
	uint32_t	cpu;

	cpu = 0;
	while (cpu < VMM_CPUS)
	{
		if ((mask >> cpu) & 1)
		{
			g_vmm.tlb.ipis++;
			if (!smp_send_ipi)
				__atomic_fetch_and(&g_vmm.tlb.pending, ~(1ull << cpu),
					__ATOMIC_SEQ_CST);
			else
				smp_send_ipi(cpu, VMM_VEC_TLB);
		}
		cpu++;
	}
}

static void	tlb_lock(void)
{
	uint32_t	ticket;

	ticket = __atomic_fetch_add(&g_vmm.tlb.ticket, 1, __ATOMIC_RELAXED);
	while (__atomic_load_n(&g_vmm.tlb.serving, __ATOMIC_ACQUIRE) != ticket)
	{
		vmm_tlb_poll();
		cpu_relax();
	}
}

static void	tlb_unlock(void)
{
	__atomic_store_n(&g_vmm.tlb.serving, g_vmm.tlb.serving + 1,
		__ATOMIC_RELEASE);
}

static void	tlb_timeout(uint64_t left)
{
	g_vmm.tlb.timeouts++;
	if (panic)
		panic("vmm: shootdown TLB sans reponse, masque %#llx", left);
	__atomic_store_n(&g_vmm.tlb.pending, 0, __ATOMIC_SEQ_CST);
}

static void	wait_acks(void)
{
	uint64_t	spins;
	uint64_t	limit;
	uint64_t	left;

	limit = g_vmm.tlb.spin_max;
	if (!limit)
		limit = TLB_SPIN_MAX;
	spins = 0;
	left = __atomic_load_n(&g_vmm.tlb.pending, __ATOMIC_ACQUIRE);
	while (left)
	{
		vmm_tlb_poll();
		cpu_relax();
		spins++;
		if (spins >= limit)
			return (tlb_timeout(left));
		if (spins % TLB_RESEND == 0)
			send_to(left);
		left = __atomic_load_n(&g_vmm.tlb.pending, __ATOMIC_ACQUIRE);
	}
}

void	vmm_tlb_shoot(t_aspace *as, uintptr_t lo, uintptr_t hi)
{
	uint64_t	mask;

	if (!g_vmm.ready || !as)
		return ;
	__atomic_thread_fence(__ATOMIC_SEQ_CST);
	mask = targets(as, vmm_cpu_id());
	if (!mask)
		return ;
	tlb_lock();
	g_vmm.tlb.req.lo = lo;
	g_vmm.tlb.req.hi = hi;
	g_vmm.tlb.shots++;
	__atomic_store_n(&g_vmm.tlb.pending, mask, __ATOMIC_SEQ_CST);
	send_to(mask);
	wait_acks();
	tlb_unlock();
}

int	vmm_tlb_init(void)
{
	if (!idt_set_handler)
		return (-1);
	return (idt_set_handler(VMM_VEC_TLB, vmm_tlb_ipi, NULL));
}
