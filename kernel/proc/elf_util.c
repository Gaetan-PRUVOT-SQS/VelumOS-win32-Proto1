#include "proc_elf.h"
#include "velum/err.h"
#include "velum/util.h"

uint64_t	elf_base_slots(const t_elfinfo *in)
{
	if (in->type != ELF_ET_DYN || in->span_hi > ELF_SPAN_MAX)
		return (0);
	return ((ELF_ASLR_HI - ELF_ASLR_LO - in->span_hi) / ELF_ASLR_ALIGN);
}

uint64_t	elf_pick_base(const t_elfinfo *in, uint64_t slot)
{
	uint64_t	slots;

	if (in->type != ELF_ET_DYN)
		return (0);
	slots = elf_base_slots(in);
	if (slot >= slots)
		slot = 0;
	return (ELF_ASLR_LO + slot * ELF_ASLR_ALIGN);
}

uint64_t	elf_mem_bytes(const t_elfinfo *in)
{
	uint64_t		total;
	uint32_t		i;
	const t_elfseg	*s;

	total = 0;
	i = 0;
	while (i < in->nseg)
	{
		s = &in->seg[i];
		total += align_up(s->vaddr + s->memsz, ELF_PAGE)
			- align_down(s->vaddr, ELF_PAGE);
		i++;
	}
	return (total);
}

int	elf_get_rela(const t_elfimg *im, const t_elfinfo *in, uint64_t i,
		t_elf64_rela *out)
{
	if (i >= in->rela_count)
		return (E_INVAL);
	return (elf_read(im, in->rela_off + i * sizeof(*out), out, sizeof(*out)));
}
