#ifndef HEAP_ST_H
# define HEAP_ST_H

# include <stddef.h>
# include <stdint.h>
# include "velum/heap.h"

# define ST_SLOTS 48
# define ST_OPS 3000
# define ST_SEED 0x4ea9c0de5eedull
# define ST_BURST 200
# define ST_LOOPS 20000
# define ST_LOOPS_LARGE 500

typedef struct s_st_case
{
	const char	*name;
	int			(*fn)(void);
}	t_st_case;

typedef struct s_st_rng
{
	uint64_t	state;
}	t_st_rng;

typedef struct s_st_state
{
	void		*p[ST_SLOTS];
	uint32_t	n[ST_SLOTS];
	t_st_rng	rng;
}	t_st_state;

void		st_fill(void *p, size_t n, uint8_t seed);
int			st_verify(const void *p, size_t n, uint8_t seed);
uint64_t	st_rand(t_st_rng *r);
uint64_t	st_live(const t_heap_stats *s);
uint64_t	st_bytes(const t_heap_stats *s);
int			st_sizes(void);
int			st_aligned(void);
int			st_tags(void);
int			st_realloc(void);
int			st_failures(void);
int			st_stress(void);
void		st_bench(void);

#endif
