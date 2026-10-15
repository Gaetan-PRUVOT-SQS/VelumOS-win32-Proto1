#include "apic_int.h"
#include "velum/cpu.h"
#include "velum/err.h"
#include "velum/irq.h"
#include "velum/klog.h"
#include "velum/msr.h"
#include "velum/vmm.h"

void	apic_eoi(void)
{
	if (lapic_ready())
		lapic_write(LAPIC_EOI, 0);
}

uint32_t	apic_id(void)
{
	uint32_t	id;

	if (!lapic_ready())
		return (0);
	id = lapic_read(LAPIC_ID);
	if (lapic_is_x2())
		return (id);
	return (id >> 24);
}

static volatile uint32_t	*lapic_map(uint64_t msr_base)
{
	uint64_t	phys;

	phys = msr_base & APIC_BASE_ADDR_MASK;
	if (acpi_info()->lapic_phys && acpi_info()->lapic_phys != phys)
		klog_warn("apic: LAPIC MADT @%#llx, MSR @%#llx : MSR retenu",
			acpi_info()->lapic_phys, phys);
	return (vmm_io_map(phys, LAPIC_MMIO_LEN, VM_R | VM_W | VM_NOCACHE));
}

static void	lapic_program(void)
{
	lapic_write(LAPIC_TPR, 0);
	lapic_write(LAPIC_LVT_TIMER, LAPIC_LVT_MASKED | VEC_TIMER);
	lapic_write(LAPIC_LVT_LINT0, LAPIC_LVT_MASKED);
	lapic_write(LAPIC_LVT_LINT1, LAPIC_LVT_MASKED);
	lapic_write(LAPIC_LVT_ERROR, LAPIC_LVT_MASKED | VEC_SPURIOUS);
	lapic_write(LAPIC_ESR, 0);
	lapic_write(LAPIC_ESR, 0);
	lapic_nmi_setup();
	lapic_write(LAPIC_SVR, LAPIC_SVR_ENABLE | VEC_SPURIOUS);
	lapic_write(LAPIC_EOI, 0);
}

int	lapic_init(void)
{
	uint64_t			base;
	bool				x2;
	volatile uint32_t	*mmio;

	base = msr_read(MSR_APIC_BASE);
	x2 = cpu_features()->x2apic || (base & APIC_BASE_EXTD);
	mmio = NULL;
	if (!(base & APIC_BASE_EN))
	{
		base |= APIC_BASE_EN;
		msr_write(MSR_APIC_BASE, base);
	}
	if (x2)
		msr_write(MSR_APIC_BASE, base | APIC_BASE_EXTD);
	else
	{
		mmio = lapic_map(base);
		if (!mmio)
			return (E_NOMEM);
	}
	lapic_set_mode(x2, mmio);
	lapic_program();
	return (E_OK);
}
