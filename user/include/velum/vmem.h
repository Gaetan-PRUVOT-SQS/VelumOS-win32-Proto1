#ifndef VMEM_H
# define VMEM_H

# include <stdint.h>
# include "velum/abi/abi_syscall.h"
# include "velum/util.h"

typedef struct s_vquery
{
	uint32_t	prot;
	uint32_t	reserved;
	uint64_t	size;
}	t_vquery;

int64_t	v_valloc(uint64_t hint_va, uint64_t len, uint32_t prot);
int		v_vfree(uint64_t va, uint64_t len);
int		v_vprotect(uint64_t va, uint64_t len, uint32_t prot);
int		v_vquery(uint64_t va, t_vquery *out);

#endif
