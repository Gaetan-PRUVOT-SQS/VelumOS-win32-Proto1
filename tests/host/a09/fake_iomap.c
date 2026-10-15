#include <stdlib.h>
#include <string.h>
#include "a09_test.h"
#include "fake.h"
#include "velum/vmm.h"

static t_fakemap	g_maps[FAKE_MAPS];

void	*vmm_io_map(uint64_t phys, size_t len, uint32_t flags)
{
	uint32_t	i;

	g_fake.io_map_calls++;
	g_fake.io_last_phys = phys;
	g_fake.io_last_len = len;
	g_fake.io_last_flags = flags;
	if (g_fake.io_map_fail)
		return (NULL);
	i = 0;
	while (i < FAKE_MAPS && g_maps[i].mem && g_maps[i].phys != phys)
		i++;
	if (i == FAKE_MAPS)
		return (NULL);
	if (!g_maps[i].mem)
	{
		g_maps[i].mem = malloc(len);
		memset(g_maps[i].mem, 0xff, len);
		g_maps[i].phys = phys;
		g_maps[i].len = len;
	}
	return (g_maps[i].mem);
}

void	vmm_io_unmap(void *virt, size_t len)
{
	(void)virt;
	(void)len;
	g_fake.io_unmap_calls++;
}

void	*fake_mmio(uint64_t phys)
{
	uint32_t	i;

	i = 0;
	while (i < FAKE_MAPS)
	{
		if (g_maps[i].mem && g_maps[i].phys == phys)
			return (g_maps[i].mem);
		i++;
	}
	return (NULL);
}

void	fake_mmio_reset(void)
{
	uint32_t	i;

	i = 0;
	while (i < FAKE_MAPS)
	{
		free(g_maps[i].mem);
		g_maps[i].mem = NULL;
		i++;
	}
}
