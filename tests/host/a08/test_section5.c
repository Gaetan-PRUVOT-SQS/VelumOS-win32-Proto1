#include "harness.h"
#include "fake.h"

#define ST_VA 0x50000000
#define ST_LEN 0x2000

static void	second_fil_mappe_tant_que_possible(void)
{
	t_process	*p;

	p = g_fk.cur;
	if (!mutex_trylock(&ht_of(p)->maplock))
	{
		g_fk.vmq_late = 1;
		return ;
	}
	mutex_unlock(&ht_of(p)->maplock);
	while (fk_map(g_fk.vmq_h, 0, PROT_R, 0) > 0)
		g_fk.vmq_n++;
}

static void	a2_plafond_tenu(void)
{
	t_process	*p;

	fk_reset();
	p = fk_proc_new(0);
	g_fk.cur = p;
	g_fk.vmq_h = fk_call(SYS_SECTION_CREATE, ST_LEN, PROT_R | PROT_W, 0);
	fk_map(g_fk.vmq_h, ST_VA, PROT_R, 0);
	fk_vmq_arm(second_fil_mappe_tant_que_possible, ST_VA + PAGE_SIZE, 0);
	vmm_unmap(p->aspace, ST_VA, PAGE_SIZE);
	secmaps_forget(p, ST_VA, PAGE_SIZE);
	h_eq_i64("second fil mis en attente", g_fk.vmq_late, 1);
	while (g_fk.vmq_late && fk_map(g_fk.vmq_h, 0, PROT_R, 0) > 0)
		g_fk.vmq_n++;
	h_eq_i64("plafond jamais depasse", ht_of(p)->nmaps, SEC_MAPS_MAX);
	h_eq_i64("mappages acceptes", g_fk.vmq_n, SEC_MAPS_MAX - 1);
	fk_call(SYS_CLOSE, (uint64_t)g_fk.vmq_h, 0, 0);
	fk_proc_free(p);
	h_eq_i64("frames rendues", g_fk.pmm_live, 0);
	h_eq_i64("tas rendu", fk_heap_total(), 0);
}

static void	second_fil_remplace_la_page_1(void)
{
	t_vmreq	vr;

	vmm_unmap(g_fk.cur->aspace, ST_VA, PAGE_SIZE);
	vr = (t_vmreq){ST_VA, 0x1234000, PAGE_SIZE, VM_R | VM_USER};
	vmm_map(g_fk.cur->aspace, &vr);
}

static void	retour_arriere_par_identite(void)
{
	t_process	*p;
	int64_t		h;

	fk_reset();
	p = fk_proc_new(0);
	g_fk.cur = p;
	h = fk_call(SYS_SECTION_CREATE, ST_LEN, PROT_R | PROT_W, 0);
	ht_of(p)->nmaps = SEC_MAPS_MAX;
	fk_vmq_arm(second_fil_remplace_la_page_1, ST_VA, 1);
	h_eq_i64("insertion refusee", fk_map(h, ST_VA, PROT_R, 0), E_NOMEM);
	ht_of(p)->nmaps = 0;
	h_eq_i64("page etrangere gardee", fk_count_maps(p->aspace), 1);
	g_fk.vmm_fail = 2;
	fk_vmq_arm(second_fil_remplace_la_page_1, FK_VMQ_ON_FAIL, 1);
	vmm_unmap(p->aspace, ST_VA, PAGE_SIZE);
	h_eq_i64("mappage partiel refuse", fk_map(h, ST_VA, PROT_R, 0), E_NOMEM);
	h_eq_i64("page etrangere encore la", fk_count_maps(p->aspace), 1);
	vmm_unmap(p->aspace, ST_VA, PAGE_SIZE);
	fk_call(SYS_CLOSE, (uint64_t)h, 0, 0);
	fk_proc_free(p);
	h_eq_i64("frames rendues", g_fk.pmm_live, 0);
	h_eq_i64("tas rendu", fk_heap_total(), 0);
}

int	main(void)
{
	h_begin("a08/section5");
	h_run("a2_plafond_tenu", a2_plafond_tenu);
	h_run("retour_arriere_par_identite", retour_arriere_par_identite);
	return (h_end());
}
