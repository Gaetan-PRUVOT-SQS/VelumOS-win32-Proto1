#ifndef ABI_MEMINFO_H
# define ABI_MEMINFO_H

# include <stdint.h>

# define MEMINFO_VERSION 1
# define MEMINFO_OWNERS 9

typedef struct s_meminfo
{
	uint32_t	version;
	uint32_t	page_size;
	uint64_t	span_bytes;
	uint64_t	usable_bytes;
	uint64_t	free_bytes;
	uint64_t	reserved_bytes;
	uint64_t	heap_window_bytes;
	uint64_t	heap_live_bytes;
	uint64_t	owned_pages[MEMINFO_OWNERS];
	uint64_t	alloc_calls;
	uint64_t	free_calls;
	uint64_t	fail_calls;
}	t_meminfo;

#endif
