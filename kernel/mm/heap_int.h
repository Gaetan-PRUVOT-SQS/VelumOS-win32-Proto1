#ifndef HEAP_INT_H
# define HEAP_INT_H

# include <stddef.h>
# include <stdint.h>
# include "heap_arena.h"
# include "heap_cfg.h"
# include "heap_large.h"
# include "slab.h"
# include "velum/heap.h"
# include "velum/sync.h"
# include "velum/util.h"

typedef struct s_heap
{
	t_heap_layout	lay;
	t_arena			arena[HEAP_ARENAS];
	t_class			cls[HEAP_CLASSES];
	t_large_state	large;
	uint64_t		bits[HEAP_WORDS_ALL];
	uint64_t		heads[HEAP_WORDS_L];
	uint8_t			by16[HEAP_BY16];
	int64_t			fail_after;
	uint64_t		fail_calls;
	uint64_t		pages_mapped;
	uint64_t		unmap_refused;
	int				ready;
}	t_heap;

extern t_heap	g_heap;

int				heap_pages_layout(t_heap_layout *out);
int				heap_pages_map(uintptr_t va, size_t npages);
int				heap_pages_unmap(uintptr_t va, size_t npages);
int				heap_unmap(uintptr_t va, size_t npages);
void			heap_lock_init(t_spinlock *l, const char *name);
uint64_t		heap_lock(t_spinlock *l);
void			heap_unlock(t_spinlock *l, uint64_t flags);
_Noreturn void	heap_fault(const char *what, const void *p, const void *at);
void			heap_req_set(t_heap_req *rq, size_t size, uint32_t tag,
					const void *at);
void			*heap_alloc(const t_heap_req *rq);
void			heap_free(void *p, const void *at);
void			*heap_fail(void);
int				heap_inject_fail(void);
size_t			heap_need(size_t size);
uint32_t		heap_class_of(const t_heap_req *rq);
int				heap_locate(uintptr_t p, uint32_t *arena);
void			heap_block_info(const void *p, const void *at, t_block *out);
int				heap_block_fits(const t_block *b, size_t size);
void			heap_block_restamp(void *p, const t_block *b, size_t size);
void			*heap_resize(void *p, const t_block *b, size_t size,
					const void *at);
void			heap_dbg_fresh(void *p, size_t n);
void			heap_dbg_zone_set(void *p, uint32_t slot, size_t size);
size_t			heap_dbg_zone_bad(const void *p, uint32_t slot);
void			heap_dbg_alloc(void *p, uint32_t slot, const t_heap_req *rq);
void			heap_dbg_free(void *p, uint32_t slot, const void *at);
size_t			heap_dbg_used(const void *p, uint32_t slot);
void			heap_dbg_resize(void *p, uint32_t slot, size_t to);
size_t			heap_dbg_scan(const void *p, size_t n, uint8_t v);
int				heap_dbg_slot_bad(const void *p, uint32_t slot, int live);
void			heap_dbg_large_fill(const t_large *h);
void			heap_dbg_large_verify(const t_large *h, const void *at);
void			heap_dbg_large_resize(t_large *h, size_t to);
int				heap_dbg_large_bad(const t_large *h);
int				heap_check_large(void);

#endif
