#include "display_int.h"
#include "kfix.h"
#include "velum/err.h"

static void	privilege_checks(void)
{
	fx_display(1024, 768);
	fake_proc_set(10, 0);
	h_eq_i64("sans PF_DISPLAY", fake_sys(SYS_DISPLAY_SET_MODE, 800, 600, 32),
		E_PERM);
	fake_proc_set(10, PF_ALL & ~PF_DISPLAY);
	h_eq_i64("tous sauf PF_DISPLAY", fake_sys(SYS_DISPLAY_SET_MODE, 800, 600,
			32), E_PERM);
	g_fp.cur = NULL;
	h_eq_i64("sans processus", fake_sys(SYS_DISPLAY_SET_MODE, 800, 600, 32),
		E_PERM);
	h_eq_u64("aucune ecriture de registre", g_fd.nlog, 0);
	h_eq_u64("mode inchange", display_info()->width, 1024);
}

static void	upper_bits_rejected(void)
{
	fx_display(1024, 768);
	h_eq_i64("largeur sur 33 bits",
		fake_sys(SYS_DISPLAY_SET_MODE, (1ull << 32) | 800, 600, 32), E_INVAL);
	h_eq_i64("hauteur sur 33 bits",
		fake_sys(SYS_DISPLAY_SET_MODE, 800, (1ull << 32) | 600, 32), E_INVAL);
	h_eq_i64("bpp sur 33 bits",
		fake_sys(SYS_DISPLAY_SET_MODE, 800, 600, (1ull << 32) | 32), E_INVAL);
	h_eq_i64("tout a u64 max",
		fake_sys(SYS_DISPLAY_SET_MODE, UINT64_MAX, UINT64_MAX, UINT64_MAX),
		E_INVAL);
	h_eq_u64("aucune ecriture de registre", g_fd.nlog, 0);
}

static void	valid_and_invalid_modes(void)
{
	fx_display(1024, 768);
	h_eq_i64("800x600", fake_sys(SYS_DISPLAY_SET_MODE, 800, 600, 32), E_OK);
	h_eq_u64("largeur", display_info()->width, 800);
	h_eq_i64("bpp 24", fake_sys(SYS_DISPLAY_SET_MODE, 1024, 768, 24), E_INVAL);
	h_eq_i64("mode hors liste", fake_sys(SYS_DISPLAY_SET_MODE, 641, 480, 32),
		E_INVAL);
	h_eq_i64("retour a 1024x768", fake_sys(SYS_DISPLAY_SET_MODE, 1024, 768,
			32), E_OK);
	h_eq_u64("largeur finale", display_info()->width, 1024);
}

static void	without_display(void)
{
	fake_reset();
	display_boot_init();
	fake_proc_set(10, PF_DISPLAY);
	h_eq_i64("avec privilege, sans affichage",
		fake_sys(SYS_DISPLAY_SET_MODE, 800, 600, 32), E_NODEV);
	fake_proc_set(10, 0);
	h_eq_i64("sans privilege : refuse d'abord",
		fake_sys(SYS_DISPLAY_SET_MODE, 800, 600, 32), E_PERM);
}

int	main(void)
{
	h_begin("a13/sys_setmode");
	h_run("set_mode privileges", privilege_checks);
	h_run("set_mode bits hauts refuses", upper_bits_rejected);
	h_run("set_mode modes valides et invalides", valid_and_invalid_modes);
	h_run("set_mode sans affichage", without_display);
	return (h_end());
}
