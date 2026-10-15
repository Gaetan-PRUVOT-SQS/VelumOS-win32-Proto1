#ifndef HEAP_H
# define HEAP_H

# include <stddef.h>
# include <stdint.h>

typedef enum e_heap_tag
{
	HEAP_GENERIC = 0,
	HEAP_PROC,
	HEAP_OBJECT,
	HEAP_FS,
	HEAP_BLOCK,
	HEAP_DRIVER,
	HEAP_IPC,
	HEAP_INPUT,
	HEAP_SCHED,
	HEAP_TAGS
}	t_heap_tag;

typedef struct s_heap_stats
{
	uint64_t	bytes_live[HEAP_TAGS];
	uint64_t	allocs_live[HEAP_TAGS];
	uint64_t	alloc_calls;
	uint64_t	free_calls;
	uint64_t	fail_calls;
	uint64_t	pages_mapped;
}	t_heap_stats;

int		heap_boot_init(void);
void	*kmalloc(size_t size);
void	*kzalloc(size_t size);
void	*kmalloc_tag(size_t size, t_heap_tag tag);
void	*kcalloc(size_t n, size_t size);
void	*krealloc(void *ptr, size_t size);
void	*kmalloc_aligned(size_t size, size_t align);
void	kfree(void *ptr);
void	heap_get_stats(t_heap_stats *out);
void	heap_fail_after(int64_t n);
int		heap_check(void);
void	heap_trim(void);
int		heap_selftest(void);

#endif
