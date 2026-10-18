#include "heap_int.h"

uint32_t	heap_steps_for_ram(uint64_t ram_bytes)
{
	uint32_t	steps;

	steps = 0;
	if (ram_bytes >= HEAP_RAM_STEP_MID)
		steps++;
	if (ram_bytes >= HEAP_RAM_STEP_HIGH)
		steps++;
	return (steps);
}

void	heap_layout_for_ram(uint64_t ram_bytes, t_heap_layout *out)
{
	uint32_t	steps;

	steps = heap_steps_for_ram(ram_bytes);
	out->slab_shift = HEAP_SHIFT_KERNEL + steps;
	out->large_pages = HEAP_LARGE_KERNEL << steps;
}

uint64_t	heap_window_bytes(const t_heap_layout *lay)
{
	return (((uint64_t)HEAP_SLAB_ARENAS << lay->slab_shift)
		+ ((uint64_t)lay->large_pages << PAGE_SHIFT));
}
