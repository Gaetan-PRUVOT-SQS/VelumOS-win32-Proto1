#include "proc_elf.h"
#include "velum/err.h"
#include "velum/util.h"
#include "velum/vmm.h"

int	elf_find_seg(const t_elfinfo *in, uint64_t va, uint64_t len)
{
	uint32_t		i;
	const t_elfseg	*s;

	i = 0;
	while (i < in->nseg)
	{
		s = &in->seg[i];
		if (va >= s->vaddr && len <= s->memsz
			&& va - s->vaddr <= s->memsz - len)
			return ((int)i);
		i++;
	}
	return (-1);
}

static int	elf_span(t_elfinfo *out)
{
	const t_elfseg	*last;

	last = &out->seg[out->nseg - 1];
	out->span_lo = align_down(out->seg[0].vaddr, ELF_PAGE);
	out->span_hi = align_up(last->vaddr + last->memsz, ELF_PAGE);
	if (out->span_hi - out->span_lo > ELF_SPAN_MAX)
		return (E_INVAL);
	if (out->type == ELF_ET_DYN && out->span_hi > ELF_SPAN_MAX)
		return (E_INVAL);
	if (out->type == ELF_ET_EXEC
		&& (out->span_lo < USER_MIN || out->span_hi > ELF_ASLR_HI))
		return (E_INVAL);
	return (0);
}

static void	elf_phdr_vaddr(t_elfinfo *out)
{
	uint64_t		bytes;
	uint32_t		i;
	const t_elfseg	*s;

	bytes = (uint64_t)out->phnum * sizeof(t_elf64_phdr);
	i = 0;
	while (i < out->nseg)
	{
		s = &out->seg[i];
		if (out->phoff >= s->offset && bytes <= s->filesz
			&& out->phoff - s->offset <= s->filesz - bytes)
		{
			out->phdr_vaddr = s->vaddr + (out->phoff - s->offset);
			return ;
		}
		i++;
	}
}

static void	elf_keep_relro(t_elfinfo *out)
{
	int	i;

	if (out->relro_hi <= out->relro_lo)
		i = -1;
	else
		i = elf_find_seg(out, out->relro_lo, out->relro_hi - out->relro_lo);
	if (i < 0 || !(out->seg[i].flags & ELF_PF_W))
	{
		out->relro_lo = 0;
		out->relro_hi = 0;
	}
}

int	elf_check_final(t_elfinfo *out)
{
	int	i;

	if (out->nseg == 0 || elf_span(out) < 0)
		return (E_INVAL);
	i = elf_find_seg(out, out->entry, 1);
	if (i < 0 || !(out->seg[i].flags & ELF_PF_X))
		return (E_INVAL);
	elf_keep_relro(out);
	elf_phdr_vaddr(out);
	return (0);
}
