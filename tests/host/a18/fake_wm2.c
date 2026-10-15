#include "fake_kern.h"
#include "wmc_int.h"

void	wmsys_unmap(void *va, uint64_t len)
{
	(void)len;
	fk_unmap(va);
}

void	wmsys_close(t_handle h)
{
	if (h != 0)
		fk_close(h);
}

uint64_t	wmsys_now(void)
{
	return (g_fk.now);
}
