#include "vmm_int.h"

static uint64_t	type_bits(uint32_t type)
{
	if (type == MEM_KERNEL)
		return (PTE_P | PTE_G | g_vmm.nx);
	if (type == MEM_FRAMEBUFFER)
		return (PTE_P | PTE_W | PTE_G | g_vmm.nx | g_vmm.wc_bits);
	if (type == MEM_USABLE || type == MEM_BOOT_RECLAIM
		|| type == MEM_ACPI_RECLAIM || type == MEM_ACPI_NVS)
		return (PTE_P | PTE_W | PTE_G | g_vmm.nx);
	return (0);
}

static void	span_push(uint64_t base, uint64_t end, uint64_t bits)
{
	t_hspan	*s;

	end = align_up(min_u64(end, HHDM_MAX), PAGE_SIZE);
	base = align_down(base, PAGE_SIZE);
	if (!bits || base >= end || g_vmm.nspan >= HSPAN_MAX)
		return ;
	s = &g_vmm.hspan[g_vmm.nspan];
	s->base = base;
	s->end = end;
	s->bits = bits;
	g_vmm.nspan++;
}

static void	span_sort(void)
{
	t_hspan		cur;
	uint32_t	i;
	uint32_t	j;

	i = 1;
	while (i < g_vmm.nspan)
	{
		cur = g_vmm.hspan[i];
		j = i;
		while (j > 0 && g_vmm.hspan[j - 1].base > cur.base)
		{
			g_vmm.hspan[j] = g_vmm.hspan[j - 1];
			j--;
		}
		g_vmm.hspan[j] = cur;
		i++;
	}
}

static void	span_merge(void)
{
	t_hspan		*s;
	uint32_t	r;
	uint32_t	w;

	s = g_vmm.hspan;
	w = 0;
	r = 1;
	while (r < g_vmm.nspan)
	{
		if (s[r].base < s[w].end)
			s[r].base = s[w].end;
		if (s[r].base < s[r].end && s[r].bits == s[w].bits
			&& s[r].base == s[w].end)
			s[w].end = s[r].end;
		else if (s[r].base < s[r].end)
		{
			w++;
			s[w] = s[r];
		}
		r++;
	}
	if (g_vmm.nspan)
		g_vmm.nspan = w + 1;
}

int	vmm_hhdm_build(const t_bootinfo *bi)
{
	const t_memrange	*r;
	uint32_t			i;
	uint32_t			n;
	uint64_t			end;

	g_vmm.nspan = 0;
	if (!bi->efi)
		span_push(BIOS_AREA_LO, BIOS_AREA_HI, PTE_P | PTE_G | g_vmm.nx);
	n = (uint32_t)min_u64(bi->nranges, BOOT_MAX_RANGES);
	i = 0;
	while (i < n)
	{
		r = &bi->ranges[i];
		if (!__builtin_add_overflow(r->base, r->length, &end))
			span_push(r->base, end, type_bits(r->type));
		i++;
	}
	span_sort();
	span_merge();
	return ((int)g_vmm.nspan);
}
