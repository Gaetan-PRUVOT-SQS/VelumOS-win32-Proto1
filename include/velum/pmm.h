#ifndef PMM_H
# define PMM_H

# include <stddef.h>
# include <stdint.h>

typedef enum e_pmm_owner
{
	PMM_FREE = 0,
	PMM_KERNEL,
	PMM_HEAP,
	PMM_PAGETABLE,
	PMM_USER,
	PMM_DMA,
	PMM_STACK,
	PMM_DRIVER,
	PMM_FB,
	PMM_OWNERS
}	t_pmm_owner;

typedef struct s_pmm_stats
{
	uint64_t	total_pages;
	uint64_t	free_pages;
	uint64_t	reserved_pages;
	uint64_t	owned[PMM_OWNERS];
	uint64_t	alloc_calls;
	uint64_t	free_calls;
	uint64_t	fail_calls;
}	t_pmm_stats;

int			pmm_boot_init(void);
uint64_t	pmm_alloc(t_pmm_owner owner);
uint64_t	pmm_alloc_zero(t_pmm_owner owner);
uint64_t	pmm_alloc_pages(t_pmm_owner o, size_t n, size_t al, uint64_t max);
void		pmm_free(uint64_t phys, t_pmm_owner owner);
void		pmm_free_pages(uint64_t phys, size_t n, t_pmm_owner owner);
int			pmm_add_region(uint64_t base, uint64_t length);
void		pmm_get_stats(t_pmm_stats *out);
void		pmm_fail_after(int64_t n);
int			pmm_check(void);

#endif
