#ifndef SLAB_H
# define SLAB_H

# include <stdint.h>
# include "heap_arena.h"
# include "heap_cfg.h"
# include "velum/heap.h"

typedef struct s_slab
{
	uint32_t		magic;
	uint16_t		inuse;
	uint16_t		nobj;
	uint8_t			cls;
	uint8_t			tag;
	uint8_t			full;
	uint8_t			arena;
	uint16_t		hint;
	uint16_t		spare;
	struct s_slab	*next;
	struct s_slab	*prev;
}	t_slab;

typedef struct s_slabq
{
	t_slab	*head;
	t_slab	*tail;
}	t_slabq;

typedef struct s_walk
{
	uint64_t		slabs;
	uint64_t		empty;
	uint64_t		inuse;
	int				bad;
	int				want_full;
	uint32_t		tag;
	const t_slab	*prev;
}	t_walk;

typedef struct s_cache
{
	t_slabq		partial;
	t_slabq		full;
	uint32_t	empty;
	uint32_t	slabs;
	uint64_t	allocs;
	uint64_t	bytes;
}	t_cache;

typedef struct s_class
{
	t_spinlock	lock;
	uint32_t	size;
	uint32_t	nobj;
	uint32_t	obj_off;
	uint32_t	arena;
	uint64_t	calls_alloc;
	uint64_t	calls_free;
	t_cache		cache[HEAP_TAGS];
}	t_class;

void		slab_class_init(void);
uint32_t	slab_class_find(size_t need, size_t align);
void		slabq_push_head(t_slabq *q, t_slab *s);
void		slabq_push_tail(t_slabq *q, t_slab *s);
void		slabq_remove(t_slabq *q, t_slab *s);
void		*slab_obj_take(const t_class *c, t_slab *s);
uint32_t	slab_idx(const t_slab *s, const void *p, const void *at);
int			slab_live(const t_slab *s, uint32_t idx);
int			slab_put(t_slab *s, uint32_t idx);
void		*cache_take(t_class *c, uint32_t tag);
void		*cache_adopt(t_class *c, t_slab *s);
int			cache_release(t_class *c, t_slab *s);
t_slab		*slab_create(t_class *c, uint32_t tag);
int			slab_destroy(t_slab *s);
void		slab_revive(t_slab *s);
void		*slab_alloc(t_class *c, const t_heap_req *rq);
void		slab_free(uint32_t arena, void *p, const void *at);
t_slab		*slab_resolve(uint32_t arena, const void *p, const void *at);
void		slab_info(uint32_t arena, const void *p, const void *at,
				t_block *out);
void		slab_trim(t_class *c);
int			slab_check(const t_class *c, const t_slab *s);
void		slab_walk(const t_class *c, const t_slabq *q, uint32_t tag,
				t_walk *w);
int			slab_plausible(const t_class *c, const t_slab *s);

static inline uint32_t	slab_cookie(const t_slab *s)
{
	return ((uint32_t)((uintptr_t)s >> 4) ^ HEAP_SLAB_MAGIC);
}

static inline uint64_t	*slab_bits(const t_slab *s)
{
	return ((uint64_t *)((char *)s + sizeof(t_slab)));
}

static inline void	*slab_obj_addr(const t_class *c, const t_slab *s,
	uint32_t idx)
{
	return ((char *)s + c->obj_off + (uint64_t)idx * c->size);
}

#endif
