#include "harness.h"
#include "fake.h"

static void	map_place(void)
{
	t_process	*p;
	int64_t		h;

	fk_reset();
	p = fk_proc_new(0);
	g_fk.cur = p;
	h = fk_call(SYS_SECTION_CREATE, 3 * PAGE_SIZE, PROT_R | PROT_W, 0);
	h_eq_i64("decalage non aligne", fk_map(h, 0, PROT_R, 12), E_INVAL);
	h_eq_i64("longueur non alignee", fk_map(h, 0, PROT_R, 1), E_INVAL);
	h_eq_i64("depasse la fin", fk_map(h, 0, PROT_R, 2 * PAGE_SIZE), E_INVAL);
	h_eq_i64("debordement", fk_map(h, 0, PROT_R, ~0xfffull), E_INVAL);
	h_eq_i64("indice non aligne", fk_map(h, 0x40000001, PROT_R, 0), E_INVAL);
	h_eq_i64("adresse voulue", fk_map(h, 0x40000000, PROT_R, PAGE_SIZE),
		0x40000000);
	h_true(fk_map(h, 0x40000000, PROT_R | PROT_W, 0) == FK_VA_BASE,
		"adresse prise : autre place");
	h_eq_i64("4 pages mappees", fk_count_maps(p->aspace), 4);
	fk_proc_free(p);
	h_eq_i64("tout demappe", fk_count_maps(NULL), 0);
	h_eq_i64("frames rendues", g_fk.pmm_live, 0);
	h_eq_i64("tas rendu", fk_heap_total(), 0);
}

static void	lifetime(void)
{
	t_process	*p;
	int64_t		h;
	t_vmreq		vr;

	fk_reset();
	p = fk_proc_new(0);
	g_fk.cur = p;
	h = fk_call(SYS_SECTION_CREATE, 2 * PAGE_SIZE, PROT_R, 0);
	g_fk.vmm_fail = 2;
	h_eq_i64("vmm echoue au milieu", fk_map(h, 0, PROT_R, 0), E_NOMEM);
	h_eq_i64("rien de mappe a moitie", fk_count_maps(NULL), 0);
	fk_map(h, 0x50000000, PROT_R, 0);
	fk_call(SYS_CLOSE, (uint64_t)h, 0, 0);
	h_eq_i64("mappage garde la section", g_fk.pmm_live, 2);
	vmm_unmap(p->aspace, 0x50000000, PAGE_SIZE);
	vr = (t_vmreq){0x50000000, 0x1234000, PAGE_SIZE, VM_R | VM_USER};
	vmm_map(p->aspace, &vr);
	secmaps_cleanup(p);
	h_eq_i64("page etrangere gardee", fk_count_maps(NULL), 1);
	h_eq_i64("frames rendues", g_fk.pmm_live, 0);
	vmm_unmap(p->aspace, 0x50000000, PAGE_SIZE);
	fk_proc_free(p);
	h_eq_i64("tas rendu", fk_heap_total(), 0);
}

int	main(void)
{
	h_begin("a08/section2");
	h_run("map_place", map_place);
	h_run("lifetime", lifetime);
	return (h_end());
}
