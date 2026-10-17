#include <string.h>
#include "harness.h"
#include "help.h"

static t_wtable	g_t;

static uint64_t	bands_area(int slot, t_rect in)
{
	t_lunawin	lw;
	t_rect		b[2];
	int32_t		n;
	int32_t		i;
	uint64_t	sum;

	wh_lunawin(&g_t, slot, &lw);
	n = luna_window_glass_bands(&lw, b);
	sum = 0;
	i = 0;
	while (i < n)
	{
		sum += wst_area(rect_intersect(rect_intersect(b[i], g_t.screen), in));
		i++;
	}
	return (sum);
}

static void	setup(void)
{
	wt_init(&g_t, rect_make(0, 0, 400, 300));
	g_t.work = g_t.screen;
	g_t.w[hz_add(&g_t, WS_DEFAULT)].rect = rect_make(20, 20, 200, 150);
	g_t.w[hz_add(&g_t, WS_DEFAULT)].rect = rect_make(100, 40, 200, 150);
}

static void	areas(void)
{
	h_eq_u64("rectangle plein", wst_area(rect_make(3, 4, 10, 20)), 200);
	h_eq_u64("largeur nulle", wst_area(rect_make(3, 4, 0, 20)), 0);
	h_eq_u64("hauteur negative", wst_area(rect_make(3, 4, 10, -2)), 0);
	h_eq_u64("sans debordement 32 bits",
		wst_area(rect_make(0, 0, WS_DIM_MAX, WS_DIM_MAX)),
		(uint64_t)WS_DIM_MAX * WS_DIM_MAX);
}

static void	glass_pixels(void)
{
	t_rect	cap;

	setup();
	h_eq_u64("deux fenetres, toute la scene", wst_glass_px(&g_t, g_t.screen),
		bands_area(0, g_t.screen) + bands_area(1, g_t.screen));
	h_true(wst_glass_px(&g_t, g_t.screen) > 0, "du verre est compte");
	cap = rect_make(30, 24, 10, 6);
	h_eq_u64("rectangle dans la legende de A seule", wst_glass_px(&g_t, cap),
		60);
	h_eq_u64("hors de toute bande", wst_glass_px(&g_t,
			rect_make(30, 90, 20, 20)), 0);
	h_eq_u64("rectangle vide", wst_glass_px(&g_t, rect_make(30, 24, 0, 6)), 0);
	g_t.w[1].state = WSTATE_MIN;
	h_eq_u64("reduite : plus de verre", wst_glass_px(&g_t, g_t.screen),
		bands_area(0, g_t.screen));
	g_t.w[0].style = WS_POPUP;
	h_eq_u64("sans cadre : plus de verre", wst_glass_px(&g_t, g_t.screen), 0);
}

static void	paint_pixels(void)
{
	setup();
	h_eq_u64("une fenetre entiere", wst_paint_px(&g_t, rect_make(20, 20, 200,
				150)), 200 * 150 + 120 * 130);
	h_eq_u64("hors des fenetres", wst_paint_px(&g_t,
			rect_make(350, 250, 40, 40)), 0);
	g_t.w[0].state = WSTATE_HIDDEN;
	h_eq_u64("cachee non peinte", wst_paint_px(&g_t, g_t.screen), 200 * 150);
}

static void	accumulation(void)
{
	t_wmeter		m;
	t_wframestat	f;

	setup();
	memset(&m, 0, sizeof(m));
	memset(&f, 0, sizeof(f));
	wst_note(&f, &g_t, rect_make(30, 24, 10, 6), rect_make(20, 20, 200, 50));
	wst_note(&f, &g_t, rect_make(30, 24, 10, 6), rect_make(20, 20, 200, 50));
	h_eq_u64("deux rectangles", f.rects, 2);
	h_eq_u64("pixels sales", f.px_dirty, 120);
	h_eq_u64("pixels composes", f.px_comp, 2 * 200 * 50);
	h_eq_u64("pixels floutes comptes deux fois", f.px_glass,
		2 * wst_glass_px(&g_t, rect_make(20, 20, 200, 50)));
	f.ns = 700;
	wst_close(&m, &f);
	f.ns = 300;
	wst_close(&m, &f);
	h_eq_u64("images", m.frames, 2);
	h_eq_u64("derniere image", m.last.ns, 300);
	h_eq_u64("temps cumule", m.total.ns, 1000);
	h_eq_u64("maximum", m.ns_max, 700);
	h_eq_u64("rectangles cumules", m.total.rects, 4);
	h_eq_u64("fenetre de journal", m.window_frames, 2);
}

static void	logging(void)
{
	t_wmeter	m;
	char		line[160];
	char		small[12];
	uint32_t	n;

	memset(&m, 0, sizeof(m));
	h_true(!wst_due(&m, 5000, 1000), "premier appel : armement");
	h_true(!wst_due(&m, 5999, 1000), "periode non ecoulee");
	m.window_frames = 1;
	h_true(wst_due(&m, 6000, 1000), "periode ecoulee");
	m.window_frames = 0;
	h_true(!wst_due(&m, 9000, 1000), "rien a dire sans image");
	memset(&m, 0, sizeof(m));
	m.window.rects = 3;
	m.window.px_dirty = 100;
	m.window.px_comp = 2000;
	m.window.px_paint = 3000;
	m.window.px_glass = 400;
	m.window.ns = 12345;
	m.window_frames = 2;
	m.ns_max = 9000;
	m.window.overflow = 1;
	n = wst_format(&m, line, sizeof(line));
	h_eq_str("ligne", line, "winsrv: mesure images=2 rects=3 sales=100 "
		"composes=2000 peints=3000 floutes=400 ns=12345 max=9000 repli=1");
	h_eq_u64("longueur rendue", n, strlen(line));
	n = wst_format(&m, small, sizeof(small));
	h_eq_u64("tampon court : tronque", n, sizeof(small) - 1);
	h_eq_u64("tampon court : termine", small[sizeof(small) - 1], 0);
	h_eq_u64("tampon nul", wst_format(&m, small, 0), 0);
	wst_rearm(&m, 77);
	h_eq_u64("fenetre remise a zero", m.window_frames + m.window.px_comp, 0);
	h_eq_u64("total conserve", m.total.rects, 0);
	h_eq_u64("echeance", m.log_at, 77);
}

int	main(void)
{
	luna_transparency_set(false);
	h_begin("a18/core_meter");
	h_run("surfaces", areas);
	h_run("pixels floutes", glass_pixels);
	h_run("pixels peints", paint_pixels);
	h_run("accumulation par image", accumulation);
	h_run("journal periodique", logging);
	return (h_end());
}
