#include "fake.h"
#include "velum/err.h"

int	pci_intx_disable(const t_pcidev *d)
{
	int	i;

	i = 0;
	while (i < FK_DEVS && (&g_fkdev[i].pci != d || !g_fkdev[i].present))
		i++;
	if (i == FK_DEVS)
		return (E_INVAL);
	g_fkdev[i].intx_calls++;
	if (g_fkdev[i].modes & FK_INTX_FAIL)
		return (E_NOTSUP);
	g_fkdev[i].cmd |= 0x400;
	g_fkdev[i].cfg[4] = (uint8_t)g_fkdev[i].cmd;
	g_fkdev[i].cfg[5] = (uint8_t)(g_fkdev[i].cmd >> 8);
	return (0);
}
