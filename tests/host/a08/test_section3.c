#include "harness.h"
#include "fake.h"

#define ST_VA 0x50000000
#define ST_LEN 0x2000
#define ST_CYCLES 2000

static void	vfree_rend_la_section(void)
{
	t_process	*p;
	int64_t		h;

	fk_reset();
	p = fk_proc_new(0);
	g_fk.cur = p;
	h = fk_call(SYS_SECTION_CREATE, ST_LEN, PROT_R | PROT_W, 0);
	h_eq_i64("mappee", fk_map(h, ST_VA, PROT_R, 0), ST_VA);
	fk_call(SYS_CLOSE, (uint64_t)h, 0, 0);
	h_eq_i64("mappage garde la section", g_fk.pmm_live, 2);
	vmm_unmap(p->aspace, ST_VA, ST_LEN);
	secmaps_forget(p, ST_VA, ST_LEN);
	h_eq_i64("frames rendues au vfree", g_fk.pmm_live, 0);
	h_eq_i64("plus de fiche", ht_of(p)->nmaps, 0);
	fk_proc_free(p);
	h_eq_i64("tas rendu", fk_heap_total(), 0);
}

static void	vfree_partiel_garde_la_section(void)
{
	t_process	*p;
	int64_t		h;

	fk_reset();
	p = fk_proc_new(0);
	g_fk.cur = p;
	h = fk_call(SYS_SECTION_CREATE, ST_LEN, PROT_R | PROT_W, 0);
	fk_map(h, ST_VA, PROT_R, 0);
	fk_call(SYS_CLOSE, (uint64_t)h, 0, 0);
	secmaps_forget(p, ST_VA, ST_LEN);
	h_eq_i64("encore mappee : fiche gardee", ht_of(p)->nmaps, 1);
	secmaps_forget(p, ST_VA + ST_LEN, PAGE_SIZE);
	secmaps_forget(p, ST_VA - PAGE_SIZE, PAGE_SIZE);
	h_eq_i64("plage voisine : fiche gardee", ht_of(p)->nmaps, 1);
	vmm_unmap(p->aspace, ST_VA, PAGE_SIZE);
	secmaps_forget(p, ST_VA, PAGE_SIZE);
	h_eq_i64("moitie mappee : frames gardees", g_fk.pmm_live, 2);
	h_eq_i64("moitie mappee : fiche gardee", ht_of(p)->nmaps, 1);
	vmm_unmap(p->aspace, ST_VA + PAGE_SIZE, PAGE_SIZE);
	secmaps_forget(p, ST_VA + PAGE_SIZE, PAGE_SIZE);
	h_eq_i64("seconde moitie : frames rendues", g_fk.pmm_live, 0);
	fk_proc_free(p);
	h_eq_i64("tas rendu", fk_heap_total(), 0);
}

static void	vfree_poignee_ouverte(void)
{
	t_process	*p;
	int64_t		h;

	fk_reset();
	p = fk_proc_new(0);
	g_fk.cur = p;
	h = fk_call(SYS_SECTION_CREATE, ST_LEN, PROT_R | PROT_W, 0);
	fk_map(h, ST_VA, PROT_R, 0);
	vmm_unmap(p->aspace, ST_VA, ST_LEN);
	secmaps_forget(p, ST_VA, ST_LEN);
	h_eq_i64("poignee ouverte : frames gardees", g_fk.pmm_live, 2);
	h_eq_i64("remappable", fk_map(h, ST_VA, PROT_R, 0), ST_VA);
	vmm_unmap(p->aspace, ST_VA, ST_LEN);
	secmaps_forget(p, ST_VA, ST_LEN);
	fk_call(SYS_CLOSE, (uint64_t)h, 0, 0);
	h_eq_i64("fermeture : frames rendues", g_fk.pmm_live, 0);
	secmaps_forget(NULL, ST_VA, ST_LEN);
	secmaps_forget(p, ST_VA, 0);
	secmaps_forget(p, ~0xfffull, ST_LEN);
	fk_proc_free(p);
	h_eq_i64("tas rendu", fk_heap_total(), 0);
}

static void	endurance_cycles(void)
{
	t_process	*p;
	int64_t		h;
	int64_t		va;
	uint32_t	i;

	fk_reset();
	p = fk_proc_new(0);
	g_fk.cur = p;
	i = 0;
	va = ST_VA;
	while (i < ST_CYCLES && va == ST_VA)
	{
		h = fk_call(SYS_SECTION_CREATE, ST_LEN, PROT_R | PROT_W, 0);
		va = fk_map(h, ST_VA, PROT_R, 0);
		fk_call(SYS_CLOSE, (uint64_t)h, 0, 0);
		vmm_unmap(p->aspace, ST_VA, ST_LEN);
		secmaps_forget(p, ST_VA, ST_LEN);
		i++;
	}
	h_eq_i64("dernier mappage reussi", va, ST_VA);
	h_eq_i64("tous les cycles joues", i, ST_CYCLES);
	h_eq_i64("aucune frame gardee", g_fk.pmm_live, 0);
	fk_proc_free(p);
	h_eq_i64("tas rendu", fk_heap_total(), 0);
}

int	main(void)
{
	h_begin("a08/section3");
	h_run("vfree_rend_la_section", vfree_rend_la_section);
	h_run("vfree_partiel_garde_la_section", vfree_partiel_garde_la_section);
	h_run("vfree_poignee_ouverte", vfree_poignee_ouverte);
	h_run("endurance_cycles", endurance_cycles);
	return (h_end());
}
