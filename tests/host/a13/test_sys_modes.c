#include "display_int.h"
#include "kfix.h"
#include "velum/err.h"

static void	modes_copy(void)
{
	t_dispmode	m[10];

	fx_display(1024, 768);
	memset(m, 0xaa, sizeof(m));
	h_eq_i64("max 8", fake_sys(SYS_DISPLAY_MODES, fx_u(m), 8, 0), 5);
	h_eq_u64("premier mode", m[0].width, 800);
	h_eq_u64("mode courant marque", m[1].flags, DISP_MODE_CURRENT);
	h_eq_u64("rien au-dela du dernier", m[5].width, 0xaaaaaaaa);
	memset(m, 0xaa, sizeof(m));
	h_eq_i64("max 3", fake_sys(SYS_DISPLAY_MODES, fx_u(m), 3, 0), 3);
	h_eq_u64("rien au-dela de max", m[3].width, 0xaaaaaaaa);
	h_eq_i64("max enorme plafonne", fake_sys(SYS_DISPLAY_MODES, fx_u(m),
			1000000, 0), 5);
	h_eq_u64("pas d'ecriture au-dela de 8 entrees", m[8].width, 0xaaaaaaaa);
}

static void	modes_arguments(void)
{
	t_dispmode	m[4];

	fx_display(1024, 768);
	h_eq_i64("max 0 : nombre", fake_sys(SYS_DISPLAY_MODES, 0, 0, 0), 5);
	h_eq_i64("max hors 32 bits", fake_sys(SYS_DISPLAY_MODES, fx_u(m),
			1ull << 32, 0), E_INVAL);
	h_eq_i64("max u64", fake_sys(SYS_DISPLAY_MODES, fx_u(m), UINT64_MAX, 0),
		E_INVAL);
	h_eq_i64("pointeur nul", fake_sys(SYS_DISPLAY_MODES, 0, 4, 0), E_FAULT);
	h_eq_i64("pointeur invalide", fake_sys(SYS_DISPLAY_MODES, FAKE_BAD_PTR, 4,
			0), E_FAULT);
	fake_proc_set(10, 0);
	h_eq_i64("sans privilege", fake_sys(SYS_DISPLAY_MODES, fx_u(m), 4, 0), 4);
}

static void	modes_without_device(void)
{
	t_dispmode	m[4];

	fx_display(1024, 768);
	g_fd.present = false;
	h_eq_i64("sans appareil", fake_sys(SYS_DISPLAY_MODES, fx_u(m), 4, 0), 1);
	h_eq_u64("le mode courant", m[0].height, 768);
	h_eq_i64("nombre seul", fake_sys(SYS_DISPLAY_MODES, 0, 0, 0), 1);
}

static void	modes_without_display(void)
{
	t_dispmode	m[4];

	fake_reset();
	display_boot_init();
	h_eq_i64("sans affichage", fake_sys(SYS_DISPLAY_MODES, fx_u(m), 4, 0),
		E_NODEV);
	h_eq_i64("max invalide : verifie d'abord", fake_sys(SYS_DISPLAY_MODES, 0,
			1ull << 40, 0), E_INVAL);
}

int	main(void)
{
	h_begin("a13/sys_modes");
	h_run("modes copie bornee", modes_copy);
	h_run("modes arguments", modes_arguments);
	h_run("modes sans appareil", modes_without_device);
	h_run("modes sans affichage", modes_without_display);
	return (h_end());
}
