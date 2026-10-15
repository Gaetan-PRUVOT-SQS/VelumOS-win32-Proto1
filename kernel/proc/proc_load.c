#include "proc_int.h"
#include "velum/err.h"
#include "velum/klog.h"
#include "velum/libk.h"
#include "velum/util.h"

int	aspace_write(t_aspace *as, uint64_t va, const void *src, uint64_t n)
{
	t_vminfo		vi;
	const uint8_t	*s;
	uint64_t		chunk;
	uint8_t			*dst;

	s = src;
	while (n > 0)
	{
		if (!vmm_query(as, align_down(va, PAGE_SIZE), &vi))
			return (E_FAULT);
		chunk = min_u64(PAGE_SIZE - (va & (PAGE_SIZE - 1)), n);
		dst = (uint8_t *)phys_to_virt(align_down(vi.pa, PAGE_SIZE))
			+ (va & (PAGE_SIZE - 1));
		memcpy(dst, s, chunk);
		s += chunk;
		va += chunk;
		n -= chunk;
	}
	return (0);
}

static int	load_seg(t_aspace *as, const t_spawnctx *c, const t_elfseg *s)
{
	uint64_t	lo;
	uint64_t	hi;
	uint32_t	vm;
	int			rc;

	lo = c->base + align_down(s->vaddr, PAGE_SIZE);
	hi = c->base + align_up(s->vaddr + s->memsz, PAGE_SIZE);
	vm = VM_USER | VM_R;
	if (s->flags & ELF_PF_W)
		vm |= VM_W;
	if (s->flags & ELF_PF_X)
		vm |= VM_X;
	rc = vmm_alloc(as, lo, hi - lo, vm);
	if (rc < 0)
		return (rc);
	return (aspace_write(as, c->base + s->vaddr,
			(const uint8_t *)c->img + s->offset, s->filesz));
}

static int	load_relocate(t_aspace *as, const t_spawnctx *c)
{
	t_elfimg		im;
	t_elf64_rela	r;
	uint64_t		k;
	uint64_t		val;
	int				rc;

	im.data = c->img;
	im.size = c->size;
	k = 0;
	rc = 0;
	while (rc == 0 && k < c->info.rela_count)
	{
		rc = elf_get_rela(&im, &c->info, k, &r);
		val = c->base + (uint64_t)r.addend;
		if (rc == 0)
			rc = aspace_write(as, c->base + r.offset, &val, sizeof(val));
		k++;
	}
	return (rc);
}

static void	load_relro(t_aspace *as, const t_spawnctx *c)
{
	static uint32_t	warned;
	uint64_t		lo;
	uint64_t		hi;

	lo = align_up(c->info.relro_lo, PAGE_SIZE);
	hi = align_down(c->info.relro_hi, PAGE_SIZE);
	if (c->info.relro_hi == 0 || hi <= lo)
		return ;
	if (vmm_protect(as, c->base + lo, hi - lo, VM_USER | VM_R) == 0)
		return ;
	if (__atomic_exchange_n(&warned, 1, __ATOMIC_RELAXED) == 0)
		klog_warn("proc: RELRO non appliqué (vmm_protect refuse)");
}

int	proc_load_image(t_spawnctx *c)
{
	uint32_t	i;
	int			rc;

	if (elf_mem_bytes(&c->info) + USTACK_SIZE > c->p->mem_limit)
		return (E_NOMEM);
	c->base = elf_pick_base(&c->info,
			proc_random_slot(elf_base_slots(&c->info)));
	c->p->image_base = c->base;
	c->p->entry = c->base + c->info.entry;
	i = 0;
	rc = 0;
	while (rc == 0 && i < c->info.nseg)
	{
		rc = load_seg(c->p->aspace, c, &c->info.seg[i]);
		i++;
	}
	if (rc == 0)
		rc = load_relocate(c->p->aspace, c);
	if (rc == 0)
		load_relro(c->p->aspace, c);
	return (rc);
}
