#include "velum/libk.h"
#include "pmm_int.h"

t_pmm	g_pmm;

void	pmm_reset(void)
{
	memset(&g_pmm, 0, sizeof(g_pmm));
}
