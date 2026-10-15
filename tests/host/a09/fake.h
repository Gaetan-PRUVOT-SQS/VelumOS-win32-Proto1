#ifndef FAKE_H
# define FAKE_H

# include <stddef.h>
# include <stdint.h>
# include "velum/acpi.h"
# include "velum/heap.h"
# include "velum/pmm.h"

# define HW_ABSENT 0
# define HW_OK 1
# define HW_BROKEN 2
# define HW_FLAKY 3
# define FAKE_LOG_MAX 16384
# define FAKE_SYSCALLS 8
# define FAKE_MAPS 80

typedef struct s_fake_hw
{
	int			mode;
	uint32_t	flaky_fails;
	uint32_t	pending;
	uint32_t	calls;
}	t_fake_hw;

typedef struct s_fakemap
{
	uint64_t	phys;
	size_t		len;
	void		*mem;
}	t_fakemap;

typedef struct s_fake_sys
{
	uint32_t	num;
	const char	*name;
}	t_fake_sys;

typedef struct s_fake
{
	uint64_t		now_ns;
	uint64_t		tsc;
	uint64_t		rng_state;
	int				cycles_on;
	uint32_t		cycles_calls;
	t_fake_hw		rdseed;
	t_fake_hw		rdrand;
	int				lock_errors;
	int				lock_depth;
	int				irq_depth;
	uint64_t		user_base;
	uint64_t		user_len;
	uint8_t			*user_mem;
	uint32_t		ncpus;
	uint32_t		nprocs;
	int				proc_list_rc;
	int				alloc_fail;
	t_pmm_stats		pmm;
	t_heap_stats	heap;
	char			brand[49];
	t_fake_sys		sys[FAKE_SYSCALLS];
	uint32_t		nsys;
	int				sys_rc;
	char			log[FAKE_LOG_MAX];
	size_t			log_len;
	int				io_map_fail;
	uint32_t		io_map_calls;
	uint32_t		io_unmap_calls;
	uint64_t		io_last_phys;
	size_t			io_last_len;
	uint32_t		io_last_flags;
	int				vec_fail;
	int				vec_next;
	uint32_t		vec_allocs;
	uint32_t		vec_frees;
	int				vec_last_freed;
	int				irq_req_rc;
	uint32_t		irq_reqs;
	uint32_t		irq_req_gsi;
	uint32_t		irq_req_flags;
	uint32_t		irq_frees;
	uint32_t		irq_freed_gsi;
	uint32_t		apic;
	int				acpi_absent;
	t_acpi_info		acpi;
}	t_fake;

extern t_fake	g_fake;

void	fake_reset(void);
void	fake_log_append(const char *fmt, ...);
int		fake_log_has(const char *needle);

#endif
