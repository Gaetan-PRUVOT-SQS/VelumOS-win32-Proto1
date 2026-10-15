#include "harness.h"
#include "lt_geo.h"

static void	expect_rect(const char *what, t_rect got, t_rect want)
{
	h_true(lt_rect_eq(got, want), what);
	if (!lt_rect_eq(got, want))
		h_eq_str(what, lt_rect_str(got), lt_rect_str(want));
}

static void	geo_case(const t_ltcase *c)
{
	t_lunawin	w;

	w = lt_win(c->outer, c->style, c->maximized);
	g_h.name = c->name;
	expect_rect("client", luna_window_client(&w), c->client);
	expect_rect("legende", luna_caption_rect(&w), c->caption);
	expect_rect("menu systeme", luna_button_rect(&w, HT_SYSMENU), c->sysmenu);
	expect_rect("reduire", luna_button_rect(&w, HT_MINBUTTON), c->btn[0]);
	expect_rect("agrandir", luna_button_rect(&w, HT_MAXBUTTON), c->btn[1]);
	expect_rect("fermer", luna_button_rect(&w, HT_CLOSE), c->btn[2]);
}

static void	geo_table(void)
{
	int	i;

	i = 0;
	while (i < g_geo_count)
	{
		geo_case(&g_geo_cases[i]);
		i++;
	}
}

static void	geo_null(void)
{
	t_rect	e;

	e = luna_window_client(NULL);
	h_true(e.w == 0 && e.h == 0, "client de NULL vide");
	e = luna_caption_rect(NULL);
	h_true(e.w == 0 && e.h == 0, "legende de NULL vide");
	e = luna_button_rect(NULL, HT_CLOSE);
	h_true(e.w == 0 && e.h == 0, "bouton de NULL vide");
	h_eq_u64("hit de NULL", luna_hit_test(NULL, lp_pt(0, 0)), HT_NOWHERE);
}

int	main(void)
{
	h_begin("a17/geo_table");
	h_run("table de geometrie", geo_table);
	h_run("arguments NULL", geo_null);
	return (h_end());
}
