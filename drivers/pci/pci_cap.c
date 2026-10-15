#include "pci_int.h"
#include "velum/err.h"

int	pcicap_next(const t_pciloc *l, uint8_t id, uint16_t prev)
{
	uint16_t	ptr;
	uint32_t	budget;
	bool		passed;
	uint8_t		cur;

	if (!(pcicfg_read(l, PCI_REG_STATUS, 2) & PCI_STATUS_CAPS))
		return (E_NOENT);
	ptr = pcicfg_read(l, PCI_REG_CAP_PTR, 1) & 0xfc;
	passed = (prev == 0);
	budget = PCI_MAX_CAPS;
	while (ptr >= 0x40 && budget)
	{
		cur = (uint8_t)pcicfg_read(l, ptr, 1);
		if (passed && cur == id)
			return (ptr);
		passed = passed || ptr == prev;
		ptr = pcicfg_read(l, ptr + 1, 1) & 0xfc;
		budget--;
	}
	return (E_NOENT);
}
