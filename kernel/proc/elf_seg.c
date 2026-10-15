#include "proc_elf.h"
#include "velum/err.h"
#include "velum/util.h"
#include "velum/vmm.h"

static int	elf_load_ok(const t_elfimg *im, const t_elf64_phdr *ph)
{
	uint64_t	end;

	if (ph->memsz == 0 || ph->filesz > ph->memsz || ph->memsz > ELF_SPAN_MAX)
		return (0);
	if (ph->offset > im->size || ph->filesz > im->size - ph->offset)
		return (0);
	if (((ph->vaddr - ph->offset) & (ELF_PAGE - 1)) != 0)
		return (0);
	if (ph->align > 1 && ((ph->align & (ph->align - 1)) != 0
			|| ph->align < ELF_PAGE))
		return (0);
	if ((ph->flags & ELF_PF_W) && (ph->flags & ELF_PF_X))
		return (0);
	if (__builtin_add_overflow(ph->vaddr, ph->memsz, &end) || end > USER_TOP)
		return (0);
	return (1);
}

static int	elf_add_load(const t_elfimg *im, const t_elf64_phdr *ph,
				t_elfinfo *out)
{
	t_elfseg	*s;
	uint64_t	prev_end;

	if (out->nseg >= ELF_SEG_MAX || !elf_load_ok(im, ph))
		return (E_INVAL);
	if (out->nseg > 0)
	{
		s = &out->seg[out->nseg - 1];
		prev_end = align_up(s->vaddr + s->memsz, ELF_PAGE);
		if (align_down(ph->vaddr, ELF_PAGE) < prev_end)
			return (E_INVAL);
	}
	s = &out->seg[out->nseg];
	s->vaddr = ph->vaddr;
	s->memsz = ph->memsz;
	s->offset = ph->offset;
	s->filesz = ph->filesz;
	s->flags = ph->flags & (ELF_PF_R | ELF_PF_W | ELF_PF_X);
	out->nseg++;
	return (0);
}

static int	elf_note_other(const t_elfimg *im, const t_elf64_phdr *ph,
				t_elfinfo *out)
{
	if (ph->type == ELF_PT_DYNAMIC)
	{
		if (out->has_dyn || ph->offset > im->size
			|| ph->filesz > im->size - ph->offset)
			return (E_INVAL);
		out->has_dyn = 1;
		out->dyn_off = ph->offset;
		out->dyn_size = ph->filesz;
	}
	else if (ph->type == ELF_PT_GNU_RELRO)
	{
		if (out->relro_hi != 0
			|| __builtin_add_overflow(ph->vaddr, ph->memsz, &out->relro_hi))
			return (E_INVAL);
		out->relro_lo = ph->vaddr;
	}
	return (0);
}

static int	elf_phdr(const t_elfimg *im, const t_elf64_phdr *ph,
				t_elfinfo *out)
{
	if (ph->type == ELF_PT_LOAD)
		return (elf_add_load(im, ph, out));
	if (ph->type == ELF_PT_INTERP)
		return (E_NOTSUP);
	if (ph->type == ELF_PT_GNU_STACK && (ph->flags & ELF_PF_X))
		return (E_NOTSUP);
	return (elf_note_other(im, ph, out));
}

int	elf_scan_phdrs(const t_elfimg *im, const t_elf64_ehdr *eh,
		t_elfinfo *out)
{
	t_elf64_phdr	ph;
	uint32_t		i;
	int				rc;

	i = 0;
	rc = 0;
	while (rc == 0 && i < eh->phnum)
	{
		rc = elf_read(im, eh->phoff + (uint64_t)i * sizeof(ph), &ph,
				sizeof(ph));
		if (rc == 0)
			rc = elf_phdr(im, &ph, out);
		i++;
	}
	return (rc);
}
