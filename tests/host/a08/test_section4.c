#include "harness.h"
#include "fake.h"

#define ST_VA 0x50000000
#define ST_LEN 0x2000

static void	second_fil_libere_la_page_2(void)
{
	t_process	*p;

	p = g_fk.cur;
	vmm_unmap(p->aspace, ST_VA + PAGE_SIZE, PAGE_SIZE);
	if (!mutex_trylock(&ht_of(p)->maplock))
	{
		g_fk.vmq_late = 1;
		return ;
	}
	mutex_unlock(&ht_of(p)->maplock);
	secmaps_forget(p, ST_VA + PAGE_SIZE, PAGE_SIZE);
}

static void	a1_vfree_croises(void)
{
	t_process	*p;
	int64_t		h;

	fk_reset();
	p = fk_proc_new(0);
	g_fk.cur = p;
	h = fk_call(SYS_SECTION_CREATE, ST_LEN, PROT_R | PROT_W, 0);
	fk_map(h, ST_VA, PROT_R, 0);
	fk_call(SYS_CLOSE, (uint64_t)h, 0, 0);
	fk_vmq_arm(second_fil_libere_la_page_2, ST_VA + PAGE_SIZE, 0);
	vmm_unmap(p->aspace, ST_VA, PAGE_SIZE);
	secmaps_forget(p, ST_VA, PAGE_SIZE);
	h_eq_i64("second fil mis en attente", g_fk.vmq_late, 1);
	if (g_fk.vmq_late)
		secmaps_forget(p, ST_VA + PAGE_SIZE, PAGE_SIZE);
	h_eq_i64("aucune fiche gardee", ht_of(p)->nmaps, 0);
	h_eq_i64("aucune frame gardee", g_fk.pmm_live, 0);
	fk_proc_free(p);
	h_eq_i64("tas rendu", fk_heap_total(), 0);
}

static void	second_fil_remappe(void)
{
	t_process	*p;

	p = g_fk.cur;
	if (!mutex_trylock(&ht_of(p)->maplock))
	{
		g_fk.vmq_late = 1;
		return ;
	}
	mutex_unlock(&ht_of(p)->maplock);
	fk_map(g_fk.vmq_h, ST_VA, PROT_R, 0);
}

static void	a3_remappage_concurrent(void)
{
	t_process	*p;

	fk_reset();
	p = fk_proc_new(0);
	g_fk.cur = p;
	g_fk.vmq_h = fk_call(SYS_SECTION_CREATE, ST_LEN, PROT_R | PROT_W, 0);
	fk_map(g_fk.vmq_h, ST_VA, PROT_R, 0);
	fk_vmq_arm(second_fil_remappe, ST_VA + PAGE_SIZE, 0);
	vmm_unmap(p->aspace, ST_VA, ST_LEN);
	secmaps_forget(p, ST_VA, ST_LEN);
	h_eq_i64("second fil mis en attente", g_fk.vmq_late, 1);
	if (g_fk.vmq_late)
		fk_map(g_fk.vmq_h, ST_VA, PROT_R, 0);
	h_eq_i64("nouveau mappage intact", fk_count_maps(p->aspace), 2);
	h_eq_i64("une seule fiche", ht_of(p)->nmaps, 1);
	fk_call(SYS_CLOSE, (uint64_t)g_fk.vmq_h, 0, 0);
	h_eq_i64("mappage garde la section", g_fk.pmm_live, 2);
	fk_proc_free(p);
	h_eq_i64("frames rendues", g_fk.pmm_live, 0);
	h_eq_i64("tas rendu", fk_heap_total(), 0);
}

int	main(void)
{
	h_begin("a08/section4");
	h_run("a1_vfree_croises", a1_vfree_croises);
	h_run("a3_remappage_concurrent", a3_remappage_concurrent);
	return (h_end());
}
