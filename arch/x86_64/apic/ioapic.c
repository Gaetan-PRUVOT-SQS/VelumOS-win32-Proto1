#include "apic_int.h"
#include "velum/err.h"
#include "velum/klog.h"
#include "velum/util.h"
#include "velum/vmm.h"

static t_ioapic_set	g_ioapics;

uint32_t	ioapic_rd(const t_ioapic_dev *d, uint32_t reg)
{
	d->base[0] = reg;
	return (d->base[IOAPIC_WIN]);
}

void	ioapic_wr(const t_ioapic_dev *d, uint32_t reg, uint32_t val)
{
	d->base[0] = reg;
	d->base[IOAPIC_WIN] = val;
}

t_ioapic_dev	*ioapic_find(uint32_t gsi, uint32_t *pin)
{
	uint32_t		k;
	t_ioapic_dev	*d;

	k = 0;
	while (k < g_ioapics.count)
	{
		d = &g_ioapics.dev[k];
		if (gsi >= d->gsi_base && gsi - d->gsi_base < d->count)
		{
			*pin = gsi - d->gsi_base;
			return (d);
		}
		k++;
	}
	return (NULL);
}

static int	ioapic_init_one(const t_ioapic_info *io, t_ioapic_dev *d)
{
	uint64_t	base;
	uint8_t		*v;
	uint32_t	pin;

	base = align_down(io->phys, PAGE_SIZE);
	v = vmm_io_map(base, PAGE_SIZE, VM_R | VM_W | VM_NOCACHE);
	if (!v || io->phys - base + IOAPIC_MMIO_LEN > PAGE_SIZE)
		return (E_NOMEM);
	d->base = (volatile uint32_t *)(v + (io->phys - base));
	d->id = io->id;
	d->gsi_base = io->gsi_base;
	d->count = ioapic_count_from_ver(ioapic_rd(d, IOAPIC_REG_VER));
	pin = 0;
	while (pin < d->count)
	{
		ioapic_wr(d, IOAPIC_REG_RED + 2 * pin, IOAPIC_RED_MASKED);
		ioapic_wr(d, IOAPIC_REG_RED + 2 * pin + 1, 0);
		pin++;
	}
	return (E_OK);
}

int	ioapic_init_all(const t_acpi_info *ai)
{
	uint32_t		k;
	t_ioapic_dev	*d;

	k = 0;
	g_ioapics.count = 0;
	while (k < ai->nioapics && k < ACPI_MAX_IOAPICS)
	{
		d = &g_ioapics.dev[g_ioapics.count];
		if (ioapic_init_one(&ai->ioapic[k], d) == E_OK)
		{
			klog_info("irq: IOAPIC id %u @%#llx, GSI %u-%u", d->id,
				ai->ioapic[k].phys, d->gsi_base, d->gsi_base + d->count - 1);
			g_ioapics.count++;
		}
		else
			klog_err("irq: IOAPIC @%#llx inutilisable", ai->ioapic[k].phys);
		k++;
	}
	return ((int)g_ioapics.count);
}
