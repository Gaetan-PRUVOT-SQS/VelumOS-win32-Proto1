#ifndef VHEAP_H
# define VHEAP_H

# include <stdint.h>

typedef struct s_vheapstats
{
	uint64_t	live_bytes;
	uint64_t	live_blocks;
	uint64_t	chunk_bytes;
	uint64_t	large_bytes;
	uint64_t	alloc_calls;
	uint64_t	free_calls;
	uint64_t	faults;
}	t_vheapstats;

void	v_heap_stats(t_vheapstats *out);

#endif
