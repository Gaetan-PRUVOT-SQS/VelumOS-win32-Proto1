#include "velum/boot.h"
#include "velum/err.h"
#include "velum/klog.h"
#include "velum/libk.h"
#include "velum/util.h"
#include "velum/vmm.h"
#include "cpu_int.h"

#define UVA_CODE 0x40000000ull
#define UVA_DATA 0x40001000ull
#define CANARY_OVERFLOW 40
#define UBSAN_SHIFT 64
#define SITE_WINDOW 256
#define RSP0_MARGIN 0x400

_Noreturn void	arch_enter_user(uint64_t entry, uint64_t sp, uint64_t arg);

typedef enum e_fault_ext
{
	XFAULT_CANARI,
	XFAULT_UBSAN,
	XFAULT_SMEP,
	XFAULT_SMAP,
	XFAULT_UMIP,
	XFAULT_COUNT
}	t_fault_ext;

static const char *const	g_xnames[XFAULT_COUNT] = {
	"canari", "ubsan", "smep", "smap", "umip"
};

static volatile int	g_sink;

static int	g_fault = FAULT_NONE;

void	cpu_fault_set(int fault)
{
	g_fault = fault;
}

static int	fault_guarded(int fault)
{
	uint64_t	guard;
	uint64_t	here;
	bool		armed;

	guard = align_up(cpu_boot_stack_low(), PAGE_SIZE);
	armed = boot_stack_guard_armed();
	if (armed)
		guard -= PAGE_SIZE;
	here = (uint64_t)__builtin_frame_address(0);
	if (here < guard + 2 * PAGE_SIZE
		|| (!armed && cpu_guard_page(guard) < 0))
	{
		klog_err("cpu: page de garde impossible pour fault=%s",
			fault_name(fault));
		return (E_NOTSUP);
	}
	kprintf("fault: page de garde %#llx\n", guard);
	if (fault == FAULT_PF)
		cpu_fault_pf(guard);
	else
		cpu_fault_stack();
	return (E_FAULT);
}

static void	fault_simple(int fault)
{
	if (fault == FAULT_DIV0)
		cpu_fault_div0();
	else if (fault == FAULT_GP)
		cpu_fault_gp();
	else if (fault == FAULT_UD)
		cpu_fault_ud();
	else if (fault == FAULT_DF)
		cpu_fault_df();
	else if (fault == FAULT_NMI)
		cpu_fault_nmi();
}

static int	fault_ext_parse(void)
{
	const char	*word;
	int			i;

	word = boot_cmdline_get("fault");
	if (!word)
		return (-1);
	i = 0;
	while (i < XFAULT_COUNT)
	{
		if (!strcmp(word, g_xnames[i]))
			return (i);
		i++;
	}
	return (-1);
}

static __attribute__((noipa)) void	canary_victim(size_t n)
{
	char			buf[16];
	volatile char	*p;
	size_t			i;

	p = buf;
	__asm__ volatile ("" : "+r" (p));
	i = 0;
	while (i < n)
	{
		p[i] = 0x41;
		i++;
	}
}

static int	fault_canary(void)
{
	volatile size_t	n;

	n = CANARY_OVERFLOW;
	kprintf("fault: canari fonction %#llx fenetre %u\n",
		(uint64_t)canary_victim, SITE_WINDOW);
	canary_victim(n);
	return (E_FAULT);
}

static __attribute__((noipa)) int	ubsan_victim(int shift)
{
	return (1 << shift);
}

static int	fault_ubsan(void)
{
	volatile int	shift;

	shift = UBSAN_SHIFT;
	kprintf("fault: ubsan fonction %#llx fenetre %u\n",
		(uint64_t)ubsan_victim, SITE_WINDOW);
	g_sink = ubsan_victim(shift);
	return (E_FAULT);
}

static int	user_page(t_aspace *as, uintptr_t va, const void *src,
				size_t n, bool exec)
{
	t_vminfo	info;
	int			rc;

	rc = vmm_alloc(as, va, PAGE_SIZE, VM_R | VM_W | VM_USER);
	if (rc < 0)
		return (rc);
	if (src)
	{
		if (!vmm_query(as, va, &info))
			return (E_FAULT);
		memcpy(phys_to_virt(info.pa), src, n);
	}
	if (!exec)
		return (E_OK);
	return (vmm_protect(as, va, PAGE_SIZE, VM_R | VM_X));
}

static int	fault_smep(void)
{
	static const uint8_t	ret_insn[1] = {0xc3};
	t_aspace				*as;
	int						rc;

	if (!(cpu_read_cr4() & CR4_SMEP))
		return (E_NOTSUP);
	as = vmm_aspace_create();
	if (!as)
		return (E_NOMEM);
	rc = user_page(as, UVA_CODE, ret_insn, sizeof(ret_insn), true);
	if (rc == 0)
	{
		vmm_switch(as);
		kprintf("fault: smep page utilisateur %#llx\n", UVA_CODE);
		kprintf("fault: rip attendu %#llx\n", UVA_CODE);
		((void (*)(void))UVA_CODE)();
		vmm_switch(NULL);
		rc = E_FAULT;
	}
	vmm_aspace_destroy(as);
	return (rc);
}

static __attribute__((noinline)) uint64_t	smap_load(uint64_t addr,
												uint64_t go, uint64_t *site)
{
	uint64_t	value;

	value = 0;
	__asm__ volatile (
		"test %[go], %[go]\n\t"
		"jz 2f\n"
		"1:\tmov (%[addr]), %[value]\n"
		"2:\tlea 1b(%%rip), %[site]"
		: [value] "+&r" (value), [site] "=r" (*site)
		: [addr] "r" (addr), [go] "r" (go)
		: "cc", "memory");
	return (value);
}

static int	fault_smap(void)
{
	t_aspace	*as;
	uint64_t	site;
	int			rc;

	if (!(cpu_read_cr4() & CR4_SMAP))
		return (E_NOTSUP);
	as = vmm_aspace_create();
	if (!as)
		return (E_NOMEM);
	rc = user_page(as, UVA_DATA, NULL, 0, false);
	if (rc == 0)
	{
		vmm_switch(as);
		__asm__ volatile ("clac" ::: "cc", "memory");
		smap_load(UVA_DATA, 0, &site);
		kprintf("fault: smap page utilisateur %#llx\n", UVA_DATA);
		kprintf("fault: rip attendu %#llx\n", site);
		g_sink = (int)smap_load(UVA_DATA, 1, &site);
		vmm_switch(NULL);
		rc = E_FAULT;
	}
	vmm_aspace_destroy(as);
	return (rc);
}

static int	fault_umip(void)
{
	uint8_t		code[16];
	uint64_t	data;
	t_aspace	*as;
	int			rc;

	if (!(cpu_read_cr4() & CR4_UMIP))
		return (E_NOTSUP);
	as = vmm_aspace_create();
	if (!as)
		return (E_NOMEM);
	data = UVA_DATA;
	memset(code, 0x90, sizeof(code));
	code[0] = 0x48;
	code[1] = 0xb8;
	memcpy(code + 2, &data, sizeof(data));
	code[10] = 0x0f;
	code[11] = 0x01;
	code[12] = 0x00;
	code[13] = 0x0f;
	code[14] = 0x0b;
	rc = user_page(as, UVA_CODE, code, sizeof(code), true);
	if (rc == 0)
		rc = user_page(as, UVA_DATA, NULL, 0, false);
	if (rc == 0)
	{
		vmm_switch(as);
		gdt_set_rsp0(((uint64_t)__builtin_frame_address(0) - RSP0_MARGIN)
			& ~0xfull);
		kprintf("fault: umip sgdt en anneau 3 a %#llx\n", UVA_CODE + 10);
		kprintf("fault: rip attendu %#llx\n", UVA_CODE + 10);
		arch_enter_user(UVA_CODE, UVA_DATA + PAGE_SIZE - 64, 0);
	}
	vmm_aspace_destroy(as);
	return (rc);
}

static int	fault_ext_run(int ext)
{
	int	rc;

	kprintf("\nfault: %s\n", g_xnames[ext]);
	rc = E_NOTSUP;
	if (ext == XFAULT_CANARI)
		rc = fault_canary();
	else if (ext == XFAULT_UBSAN)
		rc = fault_ubsan();
	else if (ext == XFAULT_SMEP)
		rc = fault_smep();
	else if (ext == XFAULT_SMAP)
		rc = fault_smap();
	else if (ext == XFAULT_UMIP)
		rc = fault_umip();
	if (rc == E_NOTSUP)
		klog_err("cpu: fault=%s indisponible sur ce CPU", g_xnames[ext]);
	else
		klog_err("cpu: fault=%s n'a pas arrêté le noyau", g_xnames[ext]);
	return (rc);
}

int	cpu_fault_run(void)
{
	int	fault;
	int	ext;

	fault = g_fault;
	if (!CPU_FAULT_INJECT)
		return (E_OK);
	if (fault == FAULT_NONE)
	{
		ext = fault_ext_parse();
		if (ext < 0)
			return (E_OK);
		return (fault_ext_run(ext));
	}
	kprintf("\nfault: %s\n", fault_name(fault));
	if (cpu_fault_rip(fault))
		kprintf("fault: rip attendu %#llx\n", cpu_fault_rip(fault));
	if (fault == FAULT_PF || fault == FAULT_STACK)
		return (fault_guarded(fault));
	fault_simple(fault);
	klog_err("cpu: fault=%s n'a pas arrêté le noyau", fault_name(fault));
	return (E_FAULT);
}
