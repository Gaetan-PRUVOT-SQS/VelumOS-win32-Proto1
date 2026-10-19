#include <string.h>
#include "harness.h"
#include "help.h"

#define W 400
#define H 300

static t_wtable	g_t;
static uint32_t	g_full[W * H];
static uint32_t	g_inc[W * H];
static uint32_t	g_scratch[4 * W * H];

static void	paint(uint32_t *px, t_rect d)
{
	t_surface	s;
	t_wcursor	c;

	gfx_surface_init(&s, px, W, H);
	c.pos = (t_point){0, 0};
	c.shape = CUR_ARROW;
	c.visible = false;
	wc_compose(&g_t, &s, c, d);
}

static t_rect	step(int kind)
{
	t_rect	old;

	wt_init(&g_t, rect_make(0, 0, W, H));
	g_t.work = g_t.screen;
	g_t.w[hz_add(&g_t, WS_DEFAULT)].rect = rect_make(20, 20, 200, 150);
	g_t.w[hz_add(&g_t, WS_DEFAULT)].rect = rect_make(100, 40, 200, 150);
	g_t.active = 1;
	paint(g_inc, g_t.screen);
	old = wsh_extent(&g_t, 1);
	if (kind == 0)
		g_t.w[1].rect = rect_make(100, 120, 200, 150);
	if (kind == 1)
		wz_raise(&g_t, 0);
	if (kind == 2)
		g_t.w[1].state = WSTATE_MIN;
	if (kind == 3)
	{
		old = wsh_extent(&g_t, 0);
		g_t.w[0].rect = rect_make(20, 20, 120, 100);
	}
	if (kind == 1)
		return (rect_intersect(wsh_extent(&g_t, 0), wsh_extent(&g_t, 1)));
	if (kind == 3)
		return (rect_union(old, wsh_extent(&g_t, 0)));
	return (rect_union(old, wsh_extent(&g_t, 1)));
}

static void	equivalence(void)
{
	t_rect	d;
	int		kind;

	kind = 0;
	while (kind < 4)
	{
		d = step(kind);
		paint(g_inc, wgl_extend(&g_t, d));
		paint(g_full, g_t.screen);
		h_true(memcmp(g_full, g_inc, sizeof(g_full)) == 0,
			"incremental etendu identique au complet");
		kind++;
	}
}

static void	sensibilite(void)
{
	t_rect	d;

	d = step(0);
	paint(g_inc, d);
	paint(g_full, g_t.screen);
	h_true(memcmp(g_full, g_inc, sizeof(g_full)) != 0,
		"sans extension, le verre diverge (le test discrimine)");
}

int	main(void)
{
	luna_transparency_set(false);
	luna_glass_scratch(g_scratch, 4 * W * H);
	h_begin("a18/core_glass_eq");
	h_run("equivalence incremental/complet : 4 transitions", equivalence);
	h_run("sensibilite : rectangle non etendu", sensibilite);
	return (h_end());
}
