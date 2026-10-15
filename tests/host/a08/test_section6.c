#include "harness.h"
#include "fake.h"

#define ST_VA 0x50000000
#define ST_LEN 0x2000

static void	demappage_refuse_a_l_insertion(void)
{
	t_process	*p;
	int64_t		h;
	t_hget		g;

	fk_reset();
	p = fk_proc_new(0);
	g_fk.cur = p;
	h = fk_call(SYS_SECTION_CREATE, ST_LEN, PROT_R | PROT_W, 0);
	sys_handle((uint64_t)h, OBJ_SECTION, 0, &g);
	obj_unref(g.obj);
	ht_of(p)->nmaps = SEC_MAPS_MAX;
	g_fk.unmap_fail = 1;
	h_eq_i64("insertion refusee", fk_map(h, ST_VA, PROT_R, 0), E_NOMEM);
	ht_of(p)->nmaps = 0;
	h_eq_i64("pages restees mappees", fk_count_maps(p->aspace), 2);
	fk_call(SYS_CLOSE, (uint64_t)h, 0, 0);
	h_eq_i64("section gardee tant que mappee", g_fk.pmm_live, 2);
	g_fk.unmap_fail = 0;
	vmm_unmap(p->aspace, ST_VA, ST_LEN);
	if (g_fk.pmm_live == 2)
		obj_unref(g.obj);
	h_eq_i64("frames rendues une fois demappe", g_fk.pmm_live, 0);
	fk_proc_free(p);
	h_eq_i64("tas rendu", fk_heap_total(), 0);
}

static void	mort_du_processus_demappage_refuse(void)
{
	t_process	*p;
	int64_t		h;

	fk_reset();
	p = fk_proc_new(0);
	g_fk.cur = p;
	h = fk_call(SYS_SECTION_CREATE, ST_LEN, PROT_R | PROT_W, 0);
	fk_map(h, ST_VA, PROT_R, 0);
	fk_call(SYS_CLOSE, (uint64_t)h, 0, 0);
	h_eq_i64("section tenue par le mappage", g_fk.pmm_live, 2);
	g_fk.unmap_fail = 1;
	secmaps_cleanup(p);
	h_eq_i64("section rendue a la mort", g_fk.pmm_live, 0);
	g_fk.unmap_fail = 0;
	vmm_unmap(p->aspace, ST_VA, ST_LEN);
	fk_proc_free(p);
	h_eq_i64("tas rendu", fk_heap_total(), 0);
}

int	main(void)
{
	h_begin("a08/section6");
	h_run("demappage_refuse_a_l_insertion", demappage_refuse_a_l_insertion);
	h_run("mort_du_processus_demappage_refuse",
		mort_du_processus_demappage_refuse);
	return (h_end());
}
