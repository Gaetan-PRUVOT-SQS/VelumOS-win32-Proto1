#ifndef FAKE_H
# define FAKE_H

# include <stdint.h>
# include "velum/pmm.h"
# include "vmm_int.h"
# include "vmm_weak.h"

# define FAKE_FRAMES 4096
# define FAKE_BASE 0x40000000ull
# define UVA 0x400000ull
# define KH_BASELINE 7
# define FAKE_CPUS 4
# define FAKE_TLB 128

typedef struct s_fake
{
	uint8_t		*arena;
	uint8_t		used[FAKE_FRAMES];
	uint8_t		owner[FAKE_FRAMES];
	uint64_t	live[PMM_OWNERS];
	uint64_t	trash[512];
	int64_t		fail_after;
	uint64_t	invlpg;
	uint64_t	cr3;
	uint64_t	invlpg_cpu[FAKE_CPUS];
	uint64_t	flush_cpu[FAKE_CPUS];
	uint64_t	ipis;
	uint64_t	stale_free;
	uint64_t	tlb_va[FAKE_CPUS][FAKE_TLB];
	uint64_t	tlb_pa[FAKE_CPUS][FAKE_TLB];
	uint32_t	ncpu;
	uint32_t	cpu;
	bool		drop_ipi;
	void		(*relax_hook)(void);
	uint32_t	cursor;
	int			errors;
}	t_fake;

extern t_fake	g_fake;

void		fake_reset(void);
t_aspace	*fake_user(void);
void		fake_clean(const char *what);
uint64_t	fake_pte(t_aspace *as, uintptr_t va);
uint8_t		*fake_ptr(t_aspace *as, uintptr_t va);
t_aspace	*fake_user_env(void);
void		fake_check_stale(uint64_t phys);
void		fake_cpu_set(uint32_t cpu);
void		fake_touch(uint32_t cpu, t_aspace *as, uintptr_t va);
bool		fake_stale(uint32_t cpu, uintptr_t va);

#endif
