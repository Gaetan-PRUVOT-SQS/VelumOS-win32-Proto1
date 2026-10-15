#include "display_int.h"
#include "kfix.h"
#include "velum/err.h"
#include "velum/vmm.h"

static void	find_free_failures(void)
{
	fx_display(1024, 768);
	g_fvmm.fail_find_mask = 0xffffffff;
	h_eq_i64("plus de place", fx_map(0), E_NOMEM);
	h_eq_i64("aucun mappage", fake_vmm_live_maps(), 0);
	g_fvmm.fail_find_mask = 0;
	h_true(fx_map(0) > 0, "reussit ensuite");
	fx_display(1024, 768);
	g_fvmm.fail_find_mask = 1;
	h_true(fx_map(0) > 0, "repli sur la recherche complete");
	h_eq_u64("deux recherches", g_fvmm.find_calls, 2);
}

static void	vmm_map_failure(void)
{
	int64_t	va;

	fx_display(1024, 768);
	g_fvmm.fail_map_mask = 1;
	h_eq_i64("vmm_map refuse", fx_map(0), E_NOMEM);
	h_eq_i64("aucun mappage", fake_vmm_live_maps(), 0);
	va = fx_map(0);
	h_true(va > 0, "reussit ensuite");
	h_eq_u64("deux demandes", g_fvmm.map_calls, 2);
	h_eq_i64("meme adresse ensuite", fx_map(0), va);
}

static void	user_unmapped_it(void)
{
	int64_t	va;

	fx_display(1024, 768);
	va = fx_map(0);
	h_eq_i64("l'appli libere sa fenetre", vmm_unmap(g_fp.proc.aspace,
			(uintptr_t)va, 3145728), E_OK);
	h_true(fx_map(0) > 0, "nouveau mappage");
	h_eq_i64("un mappage", fake_vmm_live_maps(), 1);
	h_eq_u64("deux vmm_map", g_fvmm.maps, 2);
}

static void	foreign_page_left_alone(void)
{
	int64_t	va;

	fx_display(1024, 768);
	va = fx_map(0);
	g_fvmm.map[0].pa = 0x1234000;
	h_true(fx_map(0) > 0, "nouveau mappage");
	h_eq_i64("la page etrangere reste", fake_vmm_live_maps(), 2);
	h_eq_u64("rien demappe", g_fvmm.unmaps, 0);
	h_true(va > 0, "adresse valide");
}

int	main(void)
{
	h_begin("a13/sys_mapfail");
	h_run("mapfail recherche de place", find_free_failures);
	h_run("mapfail vmm_map", vmm_map_failure);
	h_run("mapfail fenetre liberee par l'appli", user_unmapped_it);
	h_run("mapfail page etrangere", foreign_page_left_alone);
	return (h_end());
}
