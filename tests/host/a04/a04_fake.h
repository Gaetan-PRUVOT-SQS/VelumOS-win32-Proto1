#ifndef A04_FAKE_H
# define A04_FAKE_H

# include <pthread.h>
# include <stddef.h>
# include <stdint.h>
# include "harness.h"
# include "heap_int.h"

# define A04_SHIFT 22
# define A04_LARGE 8192
# define A04_DIRTY 0x5a
# define A04_SEEDS 6
# define A04_LOCK_LOOPS 20000
# define A04_META_SLOTS 192
# define A04_SCRIPT_SLOTS 12
# define A04_SHARE 256
# define A04_THREADS 4
# define A04_HOLD 1100

typedef struct s_fake_pages
{
	uint8_t			*mem;
	uint8_t			*state;
	size_t			npages;
	uint32_t		slab_shift;
	uint32_t		large_pages;
	int64_t			fail_after;
	uint32_t		unmap_fail;
	int				unmap_rc;
	int				busy;
	uint64_t		mapped;
	uint64_t		maps;
	uint64_t		unmaps;
	uint64_t		violations;
	pthread_mutex_t	lock;
}	t_fake_pages;

typedef struct s_call
{
	void	*p;
	size_t	n;
}	t_call;

typedef struct s_tagcall
{
	size_t	size;
	int		tag;
	void	*out;
}	t_tagcall;

typedef struct s_give
{
	t_arena		*a;
	uint64_t	idx;
	uint64_t	n;
}	t_give;

typedef struct s_work
{
	t_spinlock	lock;
	uint64_t	counter;
}	t_work;

typedef struct s_rng
{
	uint64_t	state;
}	t_rng;

typedef struct s_script
{
	void		*p[A04_SCRIPT_SLOTS];
	uint32_t	n[A04_SCRIPT_SLOTS];
}	t_script;

typedef struct s_meta
{
	t_rng		rng;
	void		*p[A04_META_SLOTS];
	uint32_t	n[A04_META_SLOTS];
	uint64_t	slot[A04_META_SLOTS];
	uint64_t	expect;
	uint64_t	live;
	uint64_t	sum;
}	t_meta;

typedef struct s_shared
{
	void	*slot[A04_SHARE];
}	t_shared;

typedef struct s_conc
{
	t_rng		rng;
	t_shared	*shared;
	void		*own[64];
	uint32_t	id;
	uint32_t	ops;
	uint32_t	bad;
}	t_conc;

extern t_fake_pages	g_fake;

void		fake_pages_setup(uint32_t slab_shift, uint32_t large_pages);
void		fake_pages_reset(void);
void		fake_pages_fail_after(int64_t n);
void		fake_pages_busy(int busy);
int			fake_pages_mapped_at(uintptr_t va);
int			fake_catch(void (*fn)(void *), void *arg);
const char	*fake_panic_msg(void);
int			fake_irq_depth(void);
int			fake_irq_errors(void);
int			fake_log_errors(void);
void		a04_fresh(void);
void		a04_fresh_with(uint32_t slab_shift, uint32_t large_pages);
void		a04_drain(const char *what);
void		a04_expect_panic(void (*fn)(void *), void *arg, const char *want);
uint64_t	a04_rng_next(t_rng *r);
uint64_t	a04_rng_below(t_rng *r, uint64_t n);
void		a04_fill(void *p, size_t n, uint8_t seed);
int			a04_verify(const void *p, size_t n, uint8_t seed);
uint64_t	a04_bytes(void);
uint64_t	a04_objects(void);
uint64_t	a04_slot(size_t size);
uint64_t	a04_slot_aligned(size_t size, size_t align);
void		a04_script_run(t_script *s);
void		a04_script_end(t_script *s);
uint64_t	a04_meta_run(uint64_t seed, uint64_t ops);
void		a04_meta_alloc(t_meta *m, uint32_t j);
uint32_t	a04_meta_size(t_rng *r);
void		a04_conc_run(uint64_t seed, uint32_t ops);
void		fake_klog_quiet(int quiet);
t_slab		*a04_slab_of(const void *p);
void		a04_expect_found(const char *what);
void		a04_expect_clean(const char *what);
int			a04_is_slab(const void *p);
void		a04_do_malloc(void *call);
void		a04_do_realloc(void *call);
void		a04_do_give(void *call);
void		a04_do_trim(void *call);
void		a04_do_tag(void *call);
void		a04_arena_setup(t_arena *a, uint64_t *bits, uint64_t slots);
uint32_t	a04_grab(size_t size, uint32_t tag, void **hold, uint32_t max);
void		a04_release(void **hold, uint32_t n);
void		a04_conc_step(t_conc *c);
int			fake_pages_in_state(uintptr_t va, size_t n, uint8_t want);
void		fake_pages_unmap_fail(uint32_t count, int rc);

#endif
