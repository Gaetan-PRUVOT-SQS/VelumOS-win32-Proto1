#include "display_int.h"
#include "kfix.h"
#include "velum/err.h"
#include "velum/vmm.h"

static void	privilege_and_display(void)
{
	fake_reset();
	display_boot_init();
	h_eq_i64("privilege mais sans affichage", fx_map(0), E_NODEV);
	fx_display(1024, 768);
	fake_proc_set(10, 0);
	h_eq_i64("sans PF_DISPLAY", fx_map(0), E_PERM);
	fake_proc_set(10, PF_ALL & ~PF_DISPLAY);
	h_eq_i64("tous sauf PF_DISPLAY", fx_map(0), E_PERM);
	g_fp.cur = NULL;
	h_eq_i64("sans processus", fx_map(0), E_PERM);
	h_eq_i64("aucun mappage", fake_vmm_live_maps(), 0);
	h_eq_u64("aucun appel a vmm_map", g_fvmm.maps, 0);
}

static void	first_and_second_map(void)
{
	int64_t			va;
	const t_fmap	*m;

	fx_display(1024, 768);
	va = fx_map(0);
	h_true(va >= (int64_t)USER_MIN && va <= (int64_t)(USER_TOP - 3145728),
		"adresse dans la zone utilisateur");
	h_eq_u64("alignee sur 2 Mio", (uint64_t)va % 0x200000, 0);
	h_eq_u64("graine : borne demandee", g_fp.last_bound, UMAP_RANDOM_COUNT);
	m = fx_live_map(0);
	h_eq_u64("adresse physique", m->pa, FAKE_FB_PHYS);
	h_eq_u64("longueur", m->len, 3145728);
	h_eq_u64("droits", m->flags, VM_WC | VM_USER | VM_R | VM_W);
	h_eq_i64("deuxieme appel : meme adresse", fx_map(0), va);
	h_eq_u64("pas de second vmm_map", g_fvmm.maps, 1);
	h_eq_i64("un mappage", fake_vmm_live_maps(), 1);
}

static void	hints(void)
{
	uint64_t	last;

	fx_display(1024, 768);
	last = USER_TOP - 3145728;
	h_eq_i64("indice libre", fx_map(0x40000000), 0x40000000);
	fx_display(1024, 768);
	h_eq_i64("indice non aligne", fx_map(0x40000001), E_INVAL);
	h_eq_i64("indice sous USER_MIN", fx_map(0x8000), E_INVAL);
	h_eq_i64("indice page 0", fx_map(0x1000), E_INVAL);
	h_eq_i64("indice trop haut", fx_map(last + 4096), E_INVAL);
	h_eq_i64("indice u64 max", fx_map(UINT64_MAX), E_INVAL);
	h_eq_i64("derniere adresse admise", fx_map(last), (int64_t)last);
	fx_display(1024, 768);
	h_eq_i64("USER_MIN exact", fx_map(USER_MIN), (int64_t)USER_MIN);
	h_eq_i64("aucun mappage apres refus", fake_vmm_live_maps(), 1);
}

static void	occupied_hint(void)
{
	int64_t	first;
	int64_t	other;

	fx_display(1024, 768);
	first = fx_map(0x40000000);
	fake_proc_set(12, PF_DISPLAY);
	other = fx_map(0x40000000);
	h_true(other > 0 && other != first, "indice occupe : autre adresse");
	h_eq_i64("deux mappages", fake_vmm_live_maps(), 2);
}

int	main(void)
{
	h_begin("a13/sys_map");
	h_run("map privileges et affichage", privilege_and_display);
	h_run("map premier et second appel", first_and_second_map);
	h_run("map indices valides et invalides", hints);
	h_run("map indice occupe", occupied_hint);
	return (h_end());
}
