#include "harness.h"
#include "help.h"

static t_wtable	g_t;

static bool	same(t_rect a, t_rect b)
{
	return (a.x == b.x && a.y == b.y && a.w == b.w && a.h == b.h);
}

static void	ladder(uint32_t count)
{
	uint32_t	i;
	int			slot;

	wt_init(&g_t, rect_make(0, 0, 1400, 300));
	g_t.work = g_t.screen;
	i = 0;
	while (i < count)
	{
		slot = hz_add(&g_t, WS_DEFAULT);
		g_t.w[slot].rect = rect_make(10 * (int32_t)(count - 1 - i), 0, 15,
				50);
		i++;
	}
}

static void	chain_two(void)
{
	wt_init(&g_t, rect_make(0, 0, 1024, 768));
	g_t.work = g_t.screen;
	g_t.w[hz_add(&g_t, WS_DEFAULT)].rect = rect_make(20, 20, 200, 150);
	g_t.w[hz_add(&g_t, WS_DEFAULT)].rect = rect_make(120, 40, 200, 150);
	h_true(same(wgl_extend(&g_t, rect_make(30, 25, 10, 10)),
			rect_make(20, 20, 300, 70)), "deux bandes chainees : union");
	h_true(same(wgl_extend(&g_t, rect_make(130, 45, 10, 10)),
			rect_make(20, 20, 300, 70)), "chaine dans l'autre sens");
}

static void	bounded_loop(void)
{
	t_rect	all;

	ladder(WS_WIN_MAX);
	all = rect_make(0, 0, 10 * (WS_WIN_MAX - 1) + 15, 50);
	h_true(same(wgl_extend(&g_t, rect_make(5, 5, 3, 3)), all),
		"128 fenetres en chaine : borne WS_WIN_MAX+1 atteinte, union");
	h_true(same(wgl_extend_n(&g_t, rect_make(5, 5, 3, 3), WS_WIN_MAX + 1),
			all), "limite exacte : union");
	h_true(same(wgl_extend_n(&g_t, rect_make(5, 5, 3, 3), WS_WIN_MAX),
			g_t.screen), "une passe de moins : repli ecran entier");
	h_true(same(wgl_extend_n(&g_t, rect_make(5, 5, 3, 3), 0), g_t.screen),
		"zero passe : repli ecran entier");
	ladder(10);
	h_true(same(wgl_extend_n(&g_t, rect_make(5, 5, 3, 3), 3), g_t.screen),
		"chaine de 10, 3 passes : repli ecran entier");
	h_true(same(wgl_extend_n(&g_t, rect_make(5, 5, 3, 3), 11),
			rect_make(0, 0, 105, 50)), "chaine de 10, 11 passes : union");
}

int	main(void)
{
	h_begin("a18/core_glass_chain");
	h_run("chaine de deux fenetres", chain_two);
	h_run("boucle bornee : valeurs limites des passes", bounded_loop);
	return (h_end());
}
