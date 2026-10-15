#include "kfix.h"

static void	roundtrip(void)
{
	t_kfix		f;
	uint32_t	snap[80 * 64];

	if (!kfix_open(&f, 80, 64, 80))
		return ;
	kcon_clear_all(&f.k);
	kfix_feed(&f, "hello");
	kfix_snap(&f, snap);
	kcon_cursor_show(&f.k);
	h_eq_u64("curseur : une cellule inversee", kfix_diff(&f, snap), 8 * 16);
	kcon_cursor_hide(&f.k);
	h_eq_u64("curseur : retour exact", kfix_diff(&f, snap), 0);
	kfix_close(&f);
}

static void	idempotent(void)
{
	t_kfix		f;
	uint32_t	snap[80 * 64];

	if (!kfix_open(&f, 80, 64, 80))
		return ;
	kcon_clear_all(&f.k);
	kfix_snap(&f, snap);
	kcon_cursor_show(&f.k);
	kcon_cursor_show(&f.k);
	h_eq_u64("deux affichages = un", kfix_diff(&f, snap), 8 * 16);
	kcon_cursor_hide(&f.k);
	kcon_cursor_hide(&f.k);
	h_eq_u64("deux effacements = un", kfix_diff(&f, snap), 0);
	kfix_close(&f);
}

static void	pending_wrap_position(void)
{
	t_kfix		f;
	uint32_t	snap[80 * 64];

	if (!kfix_open(&f, 80, 64, 80))
		return ;
	kcon_clear_all(&f.k);
	kfix_feed(&f, "0123456789");
	kfix_snap(&f, snap);
	kcon_cursor_show(&f.k);
	h_eq_u64("curseur borne a la derniere cellule", kfix_diff(&f, snap),
		8 * 16);
	h_eq_u64("coin bas droit inverse", f.g.px[15 * 80 + 79], 0xffffffff);
	h_eq_i64("position memorisee", f.k.cursor_x, 72);
	kfix_close(&f);
}

int	main(void)
{
	h_begin("a13/kcon_cursor");
	h_run("curseur aller-retour", roundtrip);
	h_run("curseur idempotent", idempotent);
	h_run("curseur en attente de retour a la ligne", pending_wrap_position);
	return (h_end());
}
