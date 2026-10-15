#include "harness.h"
#include "fake.h"

static void	create_partitions(void)
{
	t_process	*p;
	t_object	*s;

	fk_reset();
	p = fk_proc_new(0);
	h_eq_i64("taille 0", section_create(p, 0, PROT_R, &s), E_INVAL);
	h_eq_i64("prot 0", section_create(p, 1, 0, &s), E_INVAL);
	h_eq_i64("prot inconnue", section_create(p, 1, 8, &s), E_INVAL);
	h_eq_i64("W|X", section_create(p, 1, PROT_W | PROT_X, &s), E_INVAL);
	h_eq_i64("64 Mio + 1", section_create(p, SEC_SIZE_MAX + 1, PROT_R, &s),
		E_NOMEM);
	h_eq_i64("processus NULL", section_create(NULL, 1, PROT_R, &s), E_INVAL);
	p->mem_limit = 3 * PAGE_SIZE;
	g_fk.pages_used = 2;
	h_eq_i64("plafond depasse", section_create(p, 2 * PAGE_SIZE, PROT_R, &s),
		E_NOMEM);
	h_eq_i64("plafond juste", section_create(p, 1, PROT_R, &s), 0);
	h_eq_i64("une page", g_fk.pmm_live, 1);
	h_eq_i64("compte", p->handles->acct->bytes, PAGE_SIZE);
	obj_unref(s);
	h_eq_i64("compte rendu", p->handles->acct->bytes, 0);
	h_eq_i64("frames rendues", g_fk.pmm_live, 0);
	fk_proc_free(p);
	h_eq_i64("tas rendu", fk_heap_total(), 0);
}

static void	fail_each_rank(void)
{
	t_process	*p;
	t_object	*s;
	int			k;
	int			bad;

	fk_reset();
	p = fk_proc_new(0);
	bad = 0;
	k = 1;
	while (k <= 4)
	{
		g_fk.pmm_fail = k;
		bad += (section_create(p, 4 * PAGE_SIZE, PROT_R, &s) != E_NOMEM);
		g_fk.heap_fail = k;
		bad += (k < 4 && section_create(p, PAGE_SIZE, PROT_R, &s) != E_NOMEM);
		g_fk.heap_fail = 0;
		bad += (g_fk.pmm_live != 0 || p->handles->acct->bytes != 0);
		k++;
	}
	h_eq_i64("echec a chaque rang sans fuite", bad, 0);
	h_eq_i64("pmm rendu", g_fk.pmm_live, 0);
	fk_proc_free(p);
	h_eq_i64("tas rendu", fk_heap_total(), 0);
}

static void	map_rights(void)
{
	t_process	*p;
	int64_t		h;
	t_handle	r;

	fk_reset();
	p = fk_proc_new(0);
	g_fk.cur = p;
	h = fk_call(SYS_SECTION_CREATE, 3 * PAGE_SIZE, PROT_R | PROT_W, 0);
	handle_dup_as(p, (t_handle)h, HR_MAP_R, &r);
	h_eq_i64("W sans HR_MAP_W", fk_map(r, 0, PROT_R | PROT_W, 0), E_PERM);
	h_eq_i64("X sans HR_MAP_X", fk_map(h, 0, PROT_X, 0), E_PERM);
	h_eq_i64("prot inconnue", fk_map(h, 0, 8, 0), E_INVAL);
	h_eq_i64("prot 0", fk_map(h, 0, 0, 0), E_INVAL);
	h_eq_i64("handle d'evenement", fk_map(fk_event_in(p, HR_ALL), 0, PROT_R,
			0), E_BADF);
	h_eq_i64("R par le handle reduit", fk_map(r, 0, PROT_R, 0), FK_VA_BASE);
	fk_proc_free(p);
	h_eq_i64("tout demappe", fk_count_maps(NULL), 0);
	h_eq_i64("frames rendues", g_fk.pmm_live, 0);
	h_eq_i64("tas rendu", fk_heap_total(), 0);
}

int	main(void)
{
	h_begin("a08/section");
	h_run("create_partitions", create_partitions);
	h_run("fail_each_rank", fail_each_rank);
	h_run("map_rights", map_rights);
	return (h_end());
}
