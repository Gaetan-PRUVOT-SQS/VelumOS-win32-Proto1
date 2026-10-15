#include "limine.h"
#include "limine_req.h"
#include "velum/boot.h"

static uint32_t	map_type(uint64_t t)
{
	if (t == LIMINE_MEMMAP_USABLE)
		return (MEM_USABLE);
	if (t == LIMINE_MEMMAP_ACPI_RECLAIMABLE)
		return (MEM_ACPI_RECLAIM);
	if (t == LIMINE_MEMMAP_ACPI_NVS)
		return (MEM_ACPI_NVS);
	if (t == LIMINE_MEMMAP_BAD_MEMORY)
		return (MEM_BAD);
	if (t == LIMINE_MEMMAP_BOOTLOADER_RECLAIMABLE)
		return (MEM_BOOT_RECLAIM);
	if (t == LIMINE_MEMMAP_EXECUTABLE_AND_MODULES)
		return (MEM_KERNEL);
	if (t == LIMINE_MEMMAP_FRAMEBUFFER)
		return (MEM_FRAMEBUFFER);
	return (MEM_RESERVED);
}

static void	add_range(t_bootinfo *bi, const struct limine_memmap_entry *e)
{
	t_memrange	*r;
	uint32_t	type;

	type = map_type(e->type);
	r = &bi->ranges[bi->nranges];
	if (bi->nranges && r[-1].type == type && r[-1].base + r[-1].length
		== e->base)
	{
		r[-1].length += e->length;
		return ;
	}
	if (bi->nranges >= BOOT_MAX_RANGES)
		return ;
	r->base = e->base;
	r->length = e->length;
	r->type = type;
	r->reserved = 0;
	bi->nranges++;
}

int	boot_fill_memmap(t_bootinfo *bi)
{
	const struct limine_memmap_response	*m;
	uint64_t							i;

	m = lim_memmap();
	if (!m || !m->entry_count)
		return (-1);
	i = 0;
	while (i < m->entry_count)
	{
		add_range(bi, m->entries[i]);
		i++;
	}
	return (0);
}
