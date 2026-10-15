#include "display_int.h"
#include "kfix.h"
#include "velum/err.h"

static void	registration(void)
{
	fake_reset();
	display_boot_init();
	h_true(g_fp.sys[SYS_DISPLAY_INFO] == sys_display_info, "INFO");
	h_true(g_fp.sys[SYS_DISPLAY_MAP] == sys_display_map, "MAP");
	h_true(g_fp.sys[SYS_DISPLAY_SET_MODE] == sys_display_set_mode, "SET_MODE");
	h_true(g_fp.sys[SYS_DISPLAY_MODES] == sys_display_modes, "MODES");
	h_true(g_fp.sys[SYS_KCON] == sys_kcon, "KCON");
	h_true(g_fp.sys[SYS_KCON + 1] == NULL, "aucun numero voisin pris");
}

static void	privilege_and_values(void)
{
	fx_display(160, 96);
	fake_proc_set(10, 0);
	h_eq_i64("sans privilege", fake_sys(SYS_KCON, 1, 0, 0), E_PERM);
	fake_proc_set(10, PF_DISPLAY);
	h_eq_i64("valeur 2", fake_sys(SYS_KCON, 2, 0, 0), E_INVAL);
	h_eq_i64("valeur 255", fake_sys(SYS_KCON, 255, 0, 0), E_INVAL);
	h_eq_i64("valeur 2^32", fake_sys(SYS_KCON, 1ull << 32, 0, 0), E_INVAL);
	h_eq_i64("valeur u64 max", fake_sys(SYS_KCON, UINT64_MAX, 0, 0), E_INVAL);
	h_eq_i64("etat inchange", display_state(), DSP_SPLASH);
}

static void	effects(void)
{
	fx_display(160, 96);
	h_eq_i64("activation", fake_sys(SYS_KCON, 1, 0, 0), E_OK);
	h_eq_i64("console", display_state(), DSP_CONSOLE);
	h_eq_i64("coupure", fake_sys(SYS_KCON, 0, 0, 0), E_OK);
	h_eq_i64("arret", display_state(), DSP_OFF);
	h_eq_i64("coupure repetee", fake_sys(SYS_KCON, 0, 0, 0), E_OK);
	g_kcon.ready = false;
	h_eq_i64("console indisponible", fake_sys(SYS_KCON, 1, 0, 0), E_NOTSUP);
	h_eq_i64("toujours arrete", display_state(), DSP_OFF);
}

static void	without_display(void)
{
	fake_reset();
	display_boot_init();
	h_eq_i64("sans affichage", fake_sys(SYS_KCON, 1, 0, 0), E_NODEV);
	h_eq_i64("valeur invalide d'abord", fake_sys(SYS_KCON, 9, 0, 0), E_INVAL);
}

int	main(void)
{
	h_begin("a13/sys_kcon");
	h_run("kcon appels enregistres", registration);
	h_run("kcon privilege et valeurs", privilege_and_values);
	h_run("kcon effets", effects);
	h_run("kcon sans affichage", without_display);
	return (h_end());
}
