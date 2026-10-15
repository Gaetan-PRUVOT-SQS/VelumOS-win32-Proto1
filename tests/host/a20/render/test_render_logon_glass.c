#include "harness.h"
#include "velum/err.h"
#include "velum/luna.h"
#include "layout.h"
#include "render.h"
#include "logon_glass.h"

static uint32_t	g_ref[SCREEN_W * SCREEN_H];

static int32_t	rgb_sum(uint32_t c)
{
	return ((int32_t)((c >> 16) & 0xffu) + (int32_t)((c >> 8) & 0xffu)
		+ (int32_t)(c & 0xffu));
}

static uint32_t	wallpaper_at(t_point p)
{
	t_surface	ref;

	ref.px = g_ref;
	ref.w = SCREEN_W;
	ref.h = SCREEN_H;
	ref.stride = SCREEN_W;
	ref.clip = rect_make(0, 0, SCREEN_W, SCREEN_H);
	luna_logon_bg(&ref, ref.clip);
	return (g_ref[p.y * SCREEN_W + p.x]);
}

static void	logon_verre_blanc_34_sur_le_voile(void)
{
	t_logon	lg;
	t_point	p;
	int32_t	bg;
	int32_t	want;
	int32_t	got;

	render_reset(A20_FIXTURE);
	logon_boot(&lg);
	p = (t_point){lg.lay.card.x + lg.lay.card.w / 2,
		lg.lay.card.y + lg.lay.card.h - 12};
	bg = rgb_sum(wallpaper_at(p));
	want = bg + (765 - bg) * 34 / 100;
	got = rgb_sum(lg.ui.win.surface.px[p.y * lg.ui.win.surface.stride + p.x]);
	h_true(got - bg >= 10, "la carte eclaircit le voile");
	h_eq_i64("somme rvb a 16 pres de 34 % de blanc",
		lay_clamp(got, want - 16, want + 16), got);
	uiwin_close(&lg.ui);
}

static void	logon_tampon_de_verre_et_repli(void)
{
	static uint32_t	buf[LOGON_GLASS_PX];
	t_rect			card;
	size_t			need;

	card = rect_make(0, 0, LOGON_CARD_W, logon_card_h(1));
	need = luna_glass_need(card);
	h_eq_i64("tampon suffisant", logon_glass_attach(buf, LOGON_GLASS_PX,
			card), 0);
	h_eq_i64("capacite juste", logon_glass_attach(buf, need, card), 0);
	h_eq_i64("capacite moins un: repli", logon_glass_attach(buf, need - 1,
			card), E_NOMEM);
	h_eq_i64("sans tampon: repli", logon_glass_attach(NULL, need, card),
		E_NOMEM);
	card = rect_make(0, 0, LOGON_CARD_W, logon_card_h(LOGON_TILES_MAX));
	h_true(LOGON_GLASS_PX >= luna_glass_need(card),
		"le tampon couvre la carte la plus haute");
	logon_glass_close();
}

int	main(void)
{
	h_begin("a20/render-logon-glass");
	h_run("rendu logon: verre blanc 34 %", logon_verre_blanc_34_sur_le_voile);
	h_run("rendu logon: tampon de verre et repli",
		logon_tampon_de_verre_et_repli);
	return (h_end());
}
