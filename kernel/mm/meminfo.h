#ifndef MEMINFO_H
# define MEMINFO_H

# include <stdint.h>
# include "velum/abi/abi_meminfo.h"
# include "velum/pmm.h"

_Static_assert(MEMINFO_OWNERS == PMM_OWNERS, "propriétaires de pages");

void	meminfo_build(t_meminfo *out, const t_pmm_stats *ps,
			uint64_t heap_window, uint64_t heap_live);
void	meminfo_collect(t_meminfo *out);
int		meminfo_register(void);

#endif
