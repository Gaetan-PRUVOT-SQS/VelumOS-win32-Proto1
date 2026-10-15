#include "display_int.h"
#include "kfix.h"
#include "velum/err.h"

static void	null_and_empty(void)
{
	fx_display(160, 96);
	kcon_enable(true);
	kcon_write(NULL, 5);
	kcon_write("a", 0);
	h_eq_u64("aucun glyphe", g_ffont.glyphs, 0);
	h_eq_i64("toujours en console", display_state(), DSP_CONSOLE);
	h_eq_i64("sections critiques equilibrees", g_fk.irq_depth, 0);
}

static void	not_initialized(void)
{
	memset(&g_display, 0, sizeof(g_display));
	kcon_write("a", 1);
	kcon_clear();
	kcon_enable(false);
	h_true(!kcon_enabled(), "console inactive avant l'init");
	h_true(display_surface() == NULL, "pas de surface");
	h_eq_u64("aucun glyphe", g_ffont.glyphs, 0);
	h_eq_i64("sections critiques equilibrees", g_fk.irq_depth, 0);
}

static void	splash_with_lock_held(void)
{
	uint32_t	n;

	fx_display(160, 96);
	n = g_fl.calls;
	h_true(display_try_lock(), "verrou pris par le test");
	display_boot_progress(40, "x");
	h_eq_u64("progression ecartee", g_fl.calls, n);
	display_unlock();
	display_boot_progress(40, "x");
	h_eq_u64("progression normale", g_fl.calls, n + 1);
}

static void	surface_absent_without_display(void)
{
	fake_reset();
	display_boot_init();
	h_true(display_surface() == NULL, "pas de surface sans framebuffer");
	h_eq_i64("aucun mappage", fake_vmm_live_windows(), 0);
}

int	main(void)
{
	h_begin("a13/display_write");
	h_run("ecriture arguments nuls", null_and_empty);
	h_run("ecriture avant l'initialisation", not_initialized);
	h_run("ecriture progression avec verrou pris", splash_with_lock_held);
	h_run("ecriture surface absente", surface_absent_without_display);
	return (h_end());
}
