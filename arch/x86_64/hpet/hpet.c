#include "../../../kernel/time/time_int.h"
#include "../../../kernel/irq/a05_lock.h"
#include "velum/err.h"
#include "velum/io.h"
#include "velum/util.h"
#include "velum/vmm.h"

#define HPET_CAP 0x000
#define HPET_CFG 0x010
#define HPET_COUNTER 0x0f0
#define HPET_CAP_64BIT 0x2000ull
#define HPET_CFG_ENABLE 0x1ull
#define HPET_MAX_PERIOD_FS 0x05f5e100ull
#define FS_PER_S 1000000000000000ull
#define HPET_MMIO_LEN 0x400
#define HPET_PROBE_SPINS 100000u

static t_hpet	g_hpet;

t_hpet	*hpet_state(void)
{
	return (&g_hpet);
}

static uint64_t	hpet_reg(uint32_t off)
{
	return (*(volatile uint64_t *)(g_hpet.base + off));
}

static bool	hpet_ticks(void)
{
	uint32_t	spins;
	uint64_t	first;

	first = hpet_reg(HPET_COUNTER);
	spins = 0;
	while (spins < HPET_PROBE_SPINS)
	{
		if (hpet_reg(HPET_COUNTER) != first)
			return (true);
		outb(0x80, 0);
		spins++;
	}
	return (false);
}

static int	hpet_setup(void)
{
	uint64_t	period;

	period = hpet_reg(HPET_CAP) >> 32;
	if (!period || period > HPET_MAX_PERIOD_FS)
		return (E_NODEV);
	g_hpet.hz = FS_PER_S / period;
	g_hpet.wide = (hpet_reg(HPET_CAP) & HPET_CAP_64BIT) != 0;
	*(volatile uint64_t *)(g_hpet.base + HPET_CFG) = hpet_reg(HPET_CFG)
		| HPET_CFG_ENABLE;
	if (!hpet_ticks())
		return (E_NODEV);
	return (E_OK);
}

int	hpet_init(uint64_t phys)
{
	uint64_t	base;
	uint8_t		*v;

	if (!phys || (phys & (PAGE_SIZE - 1)) + HPET_MMIO_LEN > PAGE_SIZE)
		return (E_NODEV);
	base = align_down(phys, PAGE_SIZE);
	v = vmm_io_map(base, PAGE_SIZE, VM_R | VM_W | VM_NOCACHE);
	if (!v)
		return (E_NOMEM);
	g_hpet.base = v + (phys - base);
	g_hpet.lock.name = "hpet";
	if (hpet_setup() < 0)
	{
		vmm_io_unmap(v, PAGE_SIZE);
		g_hpet.base = NULL;
		return (E_NODEV);
	}
	g_hpet.present = true;
	return (E_OK);
}
