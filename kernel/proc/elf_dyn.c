#include "proc_elf.h"
#include "velum/err.h"
#include "velum/libk.h"

static int	elf_dyn_refused(int64_t tag)
{
	if (tag == ELF_DT_NEEDED || tag == ELF_DT_REL || tag == ELF_DT_RELSZ)
		return (1);
	if (tag == ELF_DT_TEXTREL || tag == ELF_DT_JMPREL)
		return (1);
	if (tag == ELF_DT_RELR || tag == ELF_DT_RELRSZ)
		return (1);
	return (0);
}

static int	elf_dyn_tag(const t_elf64_dyn *e, t_elfdyn *d)
{
	if (elf_dyn_refused(e->tag))
		return (E_NOTSUP);
	if (e->tag == ELF_DT_PLTRELSZ && e->val != 0)
		return (E_NOTSUP);
	if (e->tag == ELF_DT_FLAGS && (e->val & ELF_DF_TEXTREL))
		return (E_NOTSUP);
	if (e->tag == ELF_DT_RELA)
	{
		d->rela = e->val;
		d->has_rela = 1;
	}
	if (e->tag == ELF_DT_RELASZ)
		d->relasz = e->val;
	if (e->tag == ELF_DT_RELAENT)
		d->relaent = e->val;
	return (0);
}

static int	elf_rela_check(const t_elfinfo *in, const t_elf64_rela *r)
{
	int	i;

	if (r->info != ELF_R_RELATIVE)
		return (E_NOTSUP);
	i = elf_find_seg(in, r->offset, sizeof(uint64_t));
	if (i < 0 || (in->seg[i].flags & ELF_PF_X))
		return (E_INVAL);
	return (0);
}

static int	elf_check_relas(const t_elfimg *im, const t_elfdyn *d,
				t_elfinfo *out)
{
	int				i;
	uint64_t		k;
	t_elf64_rela	r;

	if (d->relasz == 0)
		return (0);
	if (!d->has_rela || d->relaent != sizeof(r) || d->relasz % sizeof(r))
		return (E_INVAL);
	i = elf_find_seg(out, d->rela, d->relasz);
	if (i < 0 || d->rela - out->seg[i].vaddr + d->relasz > out->seg[i].filesz)
		return (E_INVAL);
	out->rela_off = out->seg[i].offset + (d->rela - out->seg[i].vaddr);
	out->rela_count = d->relasz / sizeof(r);
	k = 0;
	while (k < out->rela_count)
	{
		i = elf_get_rela(im, out, k, &r);
		if (i == 0)
			i = elf_rela_check(out, &r);
		if (i < 0)
			return (i);
		k++;
	}
	return (0);
}

int	elf_check_dynamic(const t_elfimg *im, t_elfinfo *out)
{
	t_elfdyn	d;
	t_elf64_dyn	e;
	uint64_t	i;
	int			rc;

	memset(&d, 0, sizeof(d));
	i = 0;
	while (i < out->dyn_size / sizeof(e))
	{
		rc = elf_read(im, out->dyn_off + i * sizeof(e), &e, sizeof(e));
		if (rc < 0)
			return (rc);
		if (e.tag == ELF_DT_NULL)
			return (elf_check_relas(im, &d, out));
		rc = elf_dyn_tag(&e, &d);
		if (rc < 0)
			return (rc);
		i++;
	}
	return (E_INVAL);
}
