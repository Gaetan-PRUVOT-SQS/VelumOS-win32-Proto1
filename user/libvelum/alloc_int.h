#ifndef ALLOC_INT_H
# define ALLOC_INT_H

# include <stddef.h>
# include <stdint.h>
# include "velum/vheap.h"
# include "velum/vsync.h"

# define ALLOC_ALIGN 16
# define ALLOC_HDR 16
# define ALLOC_NCLASS 14
# define ALLOC_SMALL_MAX 2048
# define ALLOC_CHUNK 65536
# define ALLOC_MAX 0x40000000
# define ALLOC_CLS_LARGE 0xffffffffu
# define ALLOC_POISON 0xa5
# define ALLOC_POISON_CHECK 24
# define ALLOC_RECENT 8
# define ALLOC_SEED_DEFAULT 0x2545f4914f6cdd1dull
# define ALLOC_TAG_USED 0x5553u
# define ALLOC_TAG_FREE 0x4652u
# define ALLOC_FAULT_DOUBLE_FREE 1
# define ALLOC_FAULT_BAD_POINTER 2
# define ALLOC_FAULT_CORRUPT 3
# define ALLOC_FAULT_EXIT_CODE 134

typedef struct s_ablock
{
	uint64_t	tag;
	uint32_t	cls;
	uint32_t	pages;
}	t_ablock;

typedef void	(*t_allocfault)(int code);

typedef struct s_alloc
{
	t_vspin			lock;
	uint64_t		seed;
	uint8_t			*bump;
	uint8_t			*bump_end;
	t_ablock		*free_head[ALLOC_NCLASS];
	t_ablock		*recent[ALLOC_RECENT];
	uint32_t		recent_pos;
	t_allocfault	fault_hook;
	t_vheapstats	stats;
}	t_alloc;

extern t_alloc	g_alloc;

void		alloc_seed(uint64_t seed);
void		alloc_set_fault_hook(t_allocfault hook);
uint64_t	alloc_tag(const t_ablock *b, uint32_t state);
void		alloc_seal(t_ablock *b, uint32_t cls, uint32_t pages,
				uint32_t state);
uint32_t	alloc_state_of(const t_ablock *b);
void		alloc_fault(int code);
void		alloc_fault_default(int code);
int			alloc_class_of(size_t size);
size_t		alloc_class_size(uint32_t cls);
void		*alloc_small_get(uint32_t cls);
void		alloc_small_put(t_ablock *b);
void		*alloc_large_get(size_t size);
void		alloc_large_put(t_ablock *b);
int			alloc_large_recent(const void *p);
void		alloc_recent_forget(uint64_t lo, uint64_t hi);
size_t		alloc_capacity(const t_ablock *b);
t_ablock	*alloc_check(const void *p);
size_t		alloc_probe(const void *p);
void		*alloc_carve(uint32_t cls);
int			alloc_chunk_new(void);
void		*alloc_resize(void *p, size_t size);

#endif
