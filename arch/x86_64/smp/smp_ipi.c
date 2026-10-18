#include "smp_int.h"
#include "../apic/apic_int.h"
#include "velum/err.h"
#include "velum/irq.h"
#include "velum/smp.h"

static void	smp_ipi_handler(t_regs *regs, void *ctx)
{
	(void)regs;
	(void)ctx;
	__atomic_add_fetch(&g_smp.ipi[cpu_self()->id], 1, __ATOMIC_RELAXED);
	apic_eoi();
}

void	smp_ipi_install(void)
{
	idt_set_handler(VEC_IPI, smp_ipi_handler, NULL);
}

uint32_t	smp_cpu_id(void)
{
	return (cpu_self()->id);
}

uint32_t	smp_cpu_count(void)
{
	uint32_t	online;

	online = __atomic_load_n(&g_smp.online, __ATOMIC_ACQUIRE);
	if (online == 0)
		return (1);
	return (online);
}

uint32_t	smp_cpu_apic_id(uint32_t cpu)
{
	if (cpu >= smp_cpu_count())
		return (0xffffffffu);
	if (cpu == 0 && g_smp.online == 0)
		return (apic_id());
	return (g_smp.apic[cpu]);
}

uint64_t	smp_ipi_count(uint32_t cpu)
{
	if (cpu >= CPU_MAX)
		return (0);
	return (__atomic_load_n(&g_smp.ipi[cpu], __ATOMIC_RELAXED));
}

int	smp_send_ipi(uint32_t cpu, uint8_t vecteur)
{
	if (cpu >= smp_cpu_count() || vecteur < SMP_VEC_MIN)
		return (E_INVAL);
	return (lapic_icr_send(smp_cpu_apic_id(cpu),
			lapic_icr_value(LAPIC_ICR_FIXED, vecteur)));
}
