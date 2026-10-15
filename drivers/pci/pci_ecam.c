#include "pci_int.h"
#include "velum/err.h"

static bool	window_valid(const t_mcfg_entry *w)
{
	if (!w->base || w->bus_end < w->bus_start)
		return (false);
	return ((w->base & (PCI_ECAM_BUS_LEN - 1)) == 0);
}

int	pci_ecam_init(const t_mcfg_entry *win, uint32_t n)
{
	t_ecam		*e;
	uint32_t	i;

	e = &pci_state()->ecam;
	e->nwin = 0;
	e->nmap = 0;
	i = 0;
	while (win && i < n && e->nwin < ACPI_MAX_MCFG)
	{
		if (window_valid(&win[i]))
			e->win[e->nwin++] = win[i];
		i++;
	}
	if (!e->nwin)
		return (E_NODEV);
	return ((int)e->nwin);
}

uint32_t	pci_ecam_offset(const t_pciloc *l, uint16_t off)
{
	return (((uint32_t)l->dev << 15) | ((uint32_t)l->fn << 12) | off);
}

int	pci_ecam_window(uint16_t seg, uint8_t bus)
{
	const t_ecam	*e;
	uint32_t		i;

	e = &pci_state()->ecam;
	i = 0;
	while (i < e->nwin)
	{
		if (e->win[i].segment == seg && bus >= e->win[i].bus_start
			&& bus <= e->win[i].bus_end)
			return ((int)i);
		i++;
	}
	return (E_NODEV);
}
