#ifndef HEAP_LARGE_H
# define HEAP_LARGE_H

# include <stdint.h>
# include "heap_cfg.h"
# include "velum/heap.h"
# include "velum/sync.h"

typedef struct s_large
{
	uint64_t		cookie;
	uint64_t		size;
	uintptr_t		data;
	uint32_t		npages;
	uint32_t		tag;
	struct s_large	*next;
	struct s_large	*prev;
}	t_large;

typedef struct s_large_state
{
	t_spinlock	lock;
	t_large		*head;
	uint64_t	allocs[HEAP_TAGS];
	uint64_t	bytes[HEAP_TAGS];
	uint64_t	calls_alloc;
	uint64_t	calls_free;
}	t_large_state;

void	*large_alloc(const t_heap_req *rq);
void	large_head_set(uint64_t page, int on);
int		large_head_test(uint64_t page);
void	large_free(void *p, const void *at);
void	large_info(const void *p, const void *at, t_block *out);
t_large	*large_resolve(const void *p, const void *at);
int		large_check(const t_large *h);
int		large_release(t_large *h);
void	large_revive(t_large *h);
void	large_publish(t_large *h);

#endif
