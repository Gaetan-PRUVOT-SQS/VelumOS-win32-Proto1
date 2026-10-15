#ifndef HEAP_ARENA_H
# define HEAP_ARENA_H

# include <stdint.h>
# include "heap_cfg.h"
# include "velum/sync.h"

# define ARENA_NONE 0xffffffffffffffffull

typedef struct s_arena_cfg
{
	uint64_t	*bits;
	uintptr_t	base;
	uint64_t	slots;
	uint32_t	shift;
	const char	*name;
}	t_arena_cfg;

typedef struct s_arena
{
	t_spinlock	lock;
	uint64_t	*bits;
	uintptr_t	base;
	uint64_t	slots;
	uint64_t	taken;
	uint64_t	hint;
	uint32_t	shift;
}	t_arena;

void		arena_init(t_arena *a, const t_arena_cfg *cfg);
uint64_t	arena_take(t_arena *a, uint64_t n);
void		arena_give(t_arena *a, uint64_t idx, uint64_t n);
uint64_t	arena_scan(const t_arena *a, uint64_t n);
int			arena_held(const t_arena *a, uint64_t idx, uint64_t n);

static inline uintptr_t	arena_addr(const t_arena *a, uint64_t idx)
{
	return (a->base + (idx << a->shift));
}

static inline uint64_t	arena_index(const t_arena *a, uintptr_t addr)
{
	return ((addr - a->base) >> a->shift);
}

static inline int	arena_test(const t_arena *a, uint64_t idx)
{
	uint64_t	word;

	if (idx >= a->slots)
		return (0);
	word = __atomic_load_n(&a->bits[idx >> 6], __ATOMIC_ACQUIRE);
	return ((int)((word >> (idx & 63)) & 1));
}

#endif
