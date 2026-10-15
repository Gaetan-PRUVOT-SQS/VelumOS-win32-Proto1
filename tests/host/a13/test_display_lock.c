#include "display_int.h"
#include "kfix.h"
#include "velum/err.h"

static void	balanced_irq_and_lock(void)
{
	fx_display(160, 96);
	kcon_enable(true);
	kcon_write("abc", 3);
	kcon_clear();
	kcon_enable(false);
	kcon_write("def", 3);
	kcon_clear();
	h_eq_i64("sections critiques equilibrees", g_fk.irq_depth, 0);
	h_eq_u64("verrou libere", g_display.busy, 0);
}

static void	reentrancy_drops_writes(void)
{
	fx_display(160, 96);
	kcon_enable(true);
	h_true(display_try_lock(), "verrou pris par le test");
	kcon_write("abc", 3);
	h_eq_u64("ecriture ecartee quand le verrou est pris", g_ffont.glyphs, 0);
	h_eq_i64("sections critiques equilibrees", g_fk.irq_depth, 0);
	h_eq_u64("verrou toujours au test", g_display.busy, 1);
	display_unlock();
	kcon_write("abc", 3);
	h_eq_u64("ecriture normale ensuite", g_ffont.glyphs, 3);
}

static void	disable_with_lock_held(void)
{
	fx_display(160, 96);
	kcon_enable(true);
	h_true(display_try_lock(), "verrou pris par le test");
	kcon_enable(false);
	h_eq_i64("arret meme si le verrou reste pris", display_state(), DSP_OFF);
	h_eq_u64("verrou du test conserve", g_display.busy, 1);
	display_unlock();
	h_true(display_try_lock(), "verrou reprenable");
	display_unlock();
}

static void	cursor_follows_text(void)
{
	t_surface	*s;

	fx_display(160, 96);
	kcon_enable(true);
	s = display_surface();
	kcon_write("a", 1);
	h_eq_u64("curseur en colonne 1", s->px[15 * 160 + 15], 0xffffffff);
	h_eq_u64("pas de curseur en colonne 0", s->px[15 * 160 + 7], 0xff000000);
	kcon_write("b", 1);
	h_eq_u64("ancien curseur efface", s->px[15 * 160 + 15], 0xff000000);
	h_eq_u64("curseur en colonne 2", s->px[15 * 160 + 23], 0xffffffff);
}

int	main(void)
{
	h_begin("a13/display_lock");
	h_run("verrou sections critiques equilibrees", balanced_irq_and_lock);
	h_run("verrou reentrance", reentrancy_drops_writes);
	h_run("verrou arret avec verrou pris", disable_with_lock_held);
	h_run("verrou curseur derriere le texte", cursor_follows_text);
	return (h_end());
}
