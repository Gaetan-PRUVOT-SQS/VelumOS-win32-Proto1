#include "harness.h"
#include "velum/luna.h"
#include "layout.h"
#include "render.h"

static uint32_t	g_ref[SCREEN_W * SCREEN_H];

static int32_t	rgb_sum(uint32_t c)
{
	return ((int32_t)((c >> 16) & 0xffu) + (int32_t)((c >> 8) & 0xffu)
		+ (int32_t)(c & 0xffu));
}

static void	logon_puce_verre_blanc_34_sur_le_voile(void)
{
	t_surface	ref;
	t_logon		lg;
	t_point		p;
	int32_t		bg;
	int32_t		got;

	render_reset(A20_FIXTURE);
	logon_boot(&lg);
	ref = (t_surface){g_ref, SCREEN_W, SCREEN_H, SCREEN_W,
		rect_make(0, 0, SCREEN_W, SCREEN_H)};
	luna_logon_bg(&ref, ref.clip);
	p = (t_point){lg.lay.chips.x + 20, lg.lay.chips.y + 40};
	bg = rgb_sum(g_ref[p.y * SCREEN_W + p.x]);
	got = rgb_sum(lg.ui.win.surface.px[p.y * lg.ui.win.surface.stride + p.x]);
	h_true(got - bg >= 10, "la puce eclaircit le voile");
	h_eq_i64("puce: somme rvb a 16 pres de 34 % de blanc",
		lay_clamp(got, bg + (765 - bg) * 34 / 100 - 16,
			bg + (765 - bg) * 34 / 100 + 16), got);
	uiwin_close(&lg.ui);
}

int	main(void)
{
	h_begin("a20/render-logon-chips");
	h_run("rendu logon: puce de verre 34 %",
		logon_puce_verre_blanc_34_sur_le_voile);
	return (h_end());
}
