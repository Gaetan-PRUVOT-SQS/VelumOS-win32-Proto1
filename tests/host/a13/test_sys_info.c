#include "display_int.h"
#include "kfix.h"
#include "velum/err.h"

static void	info_ok(void)
{
	t_dispinfo	out;

	fx_display(1024, 768);
	memset(&out, 0xaa, sizeof(out));
	h_eq_i64("info", fake_sys(SYS_DISPLAY_INFO, fx_u(&out), 0, 0), E_OK);
	h_true(!memcmp(&out, display_info(), sizeof(out)), "contenu identique");
	h_eq_u64("format XRGB8888", out.format, DISP_FMT_XRGB8888);
	h_eq_i64("mutex equilibre", g_fp.locks - g_fp.unlocks, 0);
	h_eq_i64("aucune erreur de verrou", g_fp.bad_locks, 0);
}

static void	info_needs_no_privilege(void)
{
	t_dispinfo	out;

	fx_display(1024, 768);
	fake_proc_set(10, 0);
	h_eq_i64("sans privilege", fake_sys(SYS_DISPLAY_INFO, fx_u(&out), 0, 0),
		E_OK);
	g_fp.cur = NULL;
	h_eq_i64("sans processus", fake_sys(SYS_DISPLAY_INFO, fx_u(&out), 0, 0),
		E_OK);
	h_eq_u64("largeur", out.width, 1024);
}

static void	info_bad_pointer(void)
{
	fx_display(1024, 768);
	h_eq_i64("pointeur nul", fake_sys(SYS_DISPLAY_INFO, 0, 0, 0), E_FAULT);
	h_eq_i64("pointeur invalide",
		fake_sys(SYS_DISPLAY_INFO, FAKE_BAD_PTR, 0, 0), E_FAULT);
	h_eq_i64("mutex equilibre", g_fp.locks - g_fp.unlocks, 0);
}

static void	info_without_display(void)
{
	t_dispinfo	out;

	fake_reset();
	display_boot_init();
	memset(&out, 0xaa, sizeof(out));
	h_eq_i64("sans affichage", fake_sys(SYS_DISPLAY_INFO, fx_u(&out), 0, 0),
		E_NODEV);
	h_eq_u64("sortie intacte", out.width, 0xaaaaaaaa);
}

int	main(void)
{
	h_begin("a13/sys_info");
	h_run("info nominal", info_ok);
	h_run("info sans privilege", info_needs_no_privilege);
	h_run("info pointeur invalide", info_bad_pointer);
	h_run("info sans affichage", info_without_display);
	return (h_end());
}
