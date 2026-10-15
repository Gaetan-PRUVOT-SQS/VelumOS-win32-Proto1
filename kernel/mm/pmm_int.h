#ifndef PMM_INT_H
# define PMM_INT_H

# include <stddef.h>
# include <stdint.h>
# include "velum/boot.h"
# include "velum/pmm.h"
# include "velum/util.h"

# ifdef VELUM_DEBUG
#  define PMM_DEBUG 1
# else
#  define PMM_DEBUG 0
# endif

# define PMM_MARK_RESERVED 0xff
# define PMM_MARK_META 0xfe
# define PMM_LOW_END 0x100000ull
# define PMM_LOW_FRAMES 256
# define PMM_MAX_FRAMES 0x10000000000ull
# define PMM_NONE UINT64_MAX
# define PMM_POISON 0xdd

typedef enum e_pmm_fault
{
	PMM_FAULT_NONE = 0,
	PMM_FAULT_ALIGN,
	PMM_FAULT_RANGE,
	PMM_FAULT_OWNER,
	PMM_FAULT_DOUBLE
}	t_pmm_fault;

typedef struct s_pmm_req
{
	uint64_t	lo;
	uint64_t	hi;
	uint64_t	count;
	uint64_t	align;
	uint64_t	start;
}	t_pmm_req;

typedef struct s_pmm_span
{
	uint64_t	lo;
	uint64_t	hi;
}	t_pmm_span;

typedef struct s_pmm_plan
{
	uint64_t	span;
	uint64_t	meta_phys;
	uint64_t	meta_pages;
}	t_pmm_plan;

typedef struct s_pmm
{
	uint64_t	*bits;
	uint8_t		*owner;
	uint64_t	span;
	uint64_t	words;
	uint64_t	hint;
	uint64_t	hhdm;
	uint64_t	meta_phys;
	uint64_t	meta_pages;
	uint64_t	scanned;
	uint64_t	fail_left;
	uint32_t	ticket;
	uint32_t	serving;
	int			fail_armed;
	t_pmm_stats	stats;
}	t_pmm;

extern t_pmm	g_pmm;

static inline void	*pmm_virt(uint64_t phys)
{
	return ((void *)(uintptr_t)(phys + g_pmm.hhdm));
}

static inline int	pmm_bit_used(uint64_t frame)
{
	return ((int)((g_pmm.bits[frame / 64] >> (frame % 64)) & 1));
}

uint64_t	pmm_lock(void);
void		pmm_unlock(uint64_t flags);
void		pmm_reset(void);
void		pmm_bits_fill(uint64_t from, uint64_t to, int one);
uint64_t	pmm_bits_next(uint64_t from, uint64_t to, int one);
uint64_t	pmm_bits_count_free(void);
uint64_t	pmm_find_run(const t_pmm_req *q);
int			pmm_req_build(t_pmm_req *q, uint64_t n, uint64_t al, uint64_t max);
int			pmm_inject_fail(void);
uint64_t	pmm_range_end(const t_memrange *r);
int			pmm_is_pool_type(uint32_t type);
int			pmm_piece(const t_memrange *r, uint64_t floor, t_pmm_span *out);
int			pmm_ranges_sane(const t_bootinfo *bi);
uint64_t	pmm_meta_pages(uint64_t span);
int			pmm_plan_build(const t_bootinfo *bi, t_pmm_plan *plan);
void		pmm_build_tables(const t_bootinfo *bi, const t_pmm_plan *plan);
const char	*pmm_fault_name(int fault);
void		pmm_fault_report(int fault, uint64_t phys, uint64_t n, void *from);
int			pmm_selftest(void);
int			pmm_st_single(void);
int			pmm_st_pages(void);
int			pmm_st_low(void);
int			pmm_st_fail(void);
void		pmm_st_violation(void);
int			pmm_st_frame_is(uint64_t phys, uint64_t pages, uint8_t byte);

#endif
