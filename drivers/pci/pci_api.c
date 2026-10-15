#include "pci_int.h"

uint32_t	pci_count(void)
{
	return (pci_state()->count);
}

const t_pcidev	*pci_at(uint32_t i)
{
	t_pcistate	*st;

	st = pci_state();
	if (i >= st->count)
		return (NULL);
	return (&st->devs[i]);
}

const t_pcidev	*pci_find(uint16_t vendor, uint16_t device, uint32_t index)
{
	const t_pcistate	*st;
	uint32_t			i;

	st = pci_state();
	i = 0;
	while (i < st->count)
	{
		if (st->devs[i].vendor == vendor && st->devs[i].device == device)
		{
			if (index == 0)
				return (&st->devs[i]);
			index--;
		}
		i++;
	}
	return (NULL);
}

const t_pcidev	*pci_find_class(uint8_t cls, uint8_t sub, uint32_t index)
{
	const t_pcistate	*st;
	uint32_t			i;

	st = pci_state();
	i = 0;
	while (i < st->count)
	{
		if (st->devs[i].class_code == cls && st->devs[i].subclass == sub)
		{
			if (index == 0)
				return (&st->devs[i]);
			index--;
		}
		i++;
	}
	return (NULL);
}
