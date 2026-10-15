#include "a07_fake.h"
#include "velum/vmm.h"

t_fuzz	g_fz = {0x0a07c0deull, 0, 0};

uint64_t	fz_next(void)
{
	g_fz.rng ^= g_fz.rng << 13;
	g_fz.rng ^= g_fz.rng >> 7;
	g_fz.rng ^= g_fz.rng << 17;
	return (g_fz.rng);
}

int	fz_invariants(const t_elfinfo *in, uint64_t size)
{
	uint32_t		i;
	const t_elfseg	*s;
	int				x;

	if (in->nseg == 0 || in->nseg > ELF_SEG_MAX)
		return (0);
	i = 0;
	while (i < in->nseg)
	{
		s = &in->seg[i];
		if (s->filesz > s->memsz || s->offset + s->filesz > size
			|| ((s->flags & ELF_PF_W) && (s->flags & ELF_PF_X))
			|| s->vaddr + s->memsz > USER_TOP)
			return (0);
		if (i > 0 && s->vaddr < in->seg[i - 1].vaddr + in->seg[i - 1].memsz)
			return (0);
		i++;
	}
	x = elf_find_seg(in, in->entry, 1);
	if (x < 0 || !(in->seg[x].flags & ELF_PF_X))
		return (0);
	return (in->rela_off + in->rela_count * 24 <= size
		&& in->span_hi - in->span_lo <= ELF_SPAN_MAX);
}
