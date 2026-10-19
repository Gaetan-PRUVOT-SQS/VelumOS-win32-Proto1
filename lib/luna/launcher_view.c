#include "velum/libk.h"
#include "velum/theme.h"
#include "launcher_int.h"

static const char	*g_tab_label[3] = {
	"\xc3\x89pingl\xc3\xa9" "es",
	"R\xc3\xa9" "cents",
	"Toutes les apps"
};

static void	tab_one(t_surface *s, const t_launcherlayout *l, int32_t i,
				int32_t active)
{
	t_ltext	t;
	t_rect	r;

	r = luna_launcher_tab_rect(l, i);
	t.font = FONT_UI;
	t.color = lp_pal()->muted;
	if (i == active)
	{
		t.font = FONT_UI_BOLD;
		t.color = lp_pal()->ink;
		lp_fill(s, lp_rect(r.x + LL_TITLE_PAD, l->title.y + l->title.h + 1,
				r.w - 2 * LL_TITLE_PAD, 2), lp_pal()->accent);
	}
	t.shadow = 0;
	t.text = g_tab_label[i];
	t.box = lp_rect(r.x + LL_TITLE_PAD, l->title.y, r.w - 2 * LL_TITLE_PAD,
			l->title.h);
	t.align = LA_LEFT;
	lp_text(s, &t);
}

void	lp_launch_tabs(t_surface *s, const t_launcherlayout *l, int32_t active)
{
	t_ltext	t;
	int32_t	i;

	if (active > LUNA_LV_ALL)
	{
		t.font = FONT_UI_BOLD;
		t.color = lp_pal()->ink;
		t.shadow = 0;
		t.text = "R\xc3\xa9sultats";
		t.box = lp_rect(l->title.x + LL_TITLE_PAD, l->title.y,
				l->title.w - 2 * LL_TITLE_PAD, l->title.h);
		t.align = LA_LEFT;
		lp_text(s, &t);
		return ;
	}
	i = 0;
	while (i <= LUNA_LV_ALL)
		tab_one(s, l, i++, active);
}

void	lp_launch_query(t_surface *s, const t_launcherlayout *l,
			const char *query)
{
	char	shown[LUNA_LM_QUERY_MAX + 2];
	t_ltext	t;
	t_rect	r;
	size_t	n;

	lp_launch_search(s, l);
	if (query == NULL || query[0] == '\0')
		return ;
	r = l->search;
	lp_launch_fill(s, r, r.h / 2, lp_pal()->input);
	lp_fill(s, lp_rect(r.x + r.h / 2, r.y, r.w - r.h, 1),
		lp_launch_tone(LL_INPUT_RIM_L, LL_INPUT_RIM_D));
	luna_icon(s, lp_pt(r.x + LL_SEARCH_PAD, r.y + (r.h - LL_SEARCH_ICON) / 2),
		ICON_SEARCH, LL_SEARCH_ICON);
	n = (size_t)lp_text_len(query);
	n = lp_min((int32_t)n, LUNA_LM_QUERY_MAX);
	memcpy(shown, query, n);
	shown[n] = '|';
	shown[n + 1] = '\0';
	t.font = FONT_UI;
	t.color = lp_pal()->ink;
	t.shadow = 0;
	t.text = shown;
	t.box = lp_rect(r.x + LL_SEARCH_PAD + LL_SEARCH_ICON + LL_SEARCH_PAD, r.y,
			r.w - 2 * LL_SEARCH_PAD - LL_SEARCH_ICON - LL_SEARCH_PAD_R, r.h);
	t.align = LA_LEFT;
	lp_text(s, &t);
}

static void	row_one(t_surface *s, const t_launcherlayout *l,
				const t_lmodel *m, int32_t i)
{
	const t_lmapp	*a;
	t_ltext			t;
	t_rect			r;

	a = luna_lm_row(m, i);
	if (a == NULL)
		return ;
	r = luna_launcher_row_rect(l, i);
	if (m->hot == i || m->sel == m->top + i)
		lp_launch_fill(s, r, VT_R_TILE, lp_launch_tone(LL_HOT_L, LL_HOT_D));
	luna_icon(s, lp_pt(r.x + LL_ROW_PAD, r.y + (r.h - LL_ROW_ICON) / 2),
		a->icon, LL_ROW_ICON);
	t.font = FONT_UI_BOLD;
	t.color = lp_pal()->ink;
	t.shadow = 0;
	t.text = a->label;
	t.box = lp_rect(r.x + 2 * LL_ROW_PAD + LL_ROW_ICON, r.y,
			r.w - 3 * LL_ROW_PAD - LL_ROW_ICON, r.h);
	t.align = LA_LEFT;
	lp_text(s, &t);
}

static void	rows_bar(t_surface *s, const t_launcherlayout *l,
				const t_lmodel *m)
{
	t_rect	first;
	int32_t	track;
	int32_t	thumb;
	int32_t	span;

	if ((int32_t)m->nshown <= LUNA_LM_ROWS)
		return ;
	first = luna_launcher_row_rect(l, 0);
	track = LUNA_LM_ROWS * LL_ROW_H;
	thumb = lp_max(track * LUNA_LM_ROWS / (int32_t)m->nshown, LL_BAR_MIN);
	span = (int32_t)m->nshown - LUNA_LM_ROWS;
	lp_launch_fill(s, lp_rect(l->search.x + l->search.w - LL_BAR_W, first.y,
			LL_BAR_W, track), 1, lp_launch_tone(LL_SEP_L, LL_SEP_D));
	lp_launch_fill(s, lp_rect(l->search.x + l->search.w - LL_BAR_W,
			first.y + m->top * (track - thumb) / span, LL_BAR_W, thumb), 1,
		lp_pal()->muted);
}

void	lp_launch_rows(t_surface *s, const t_launcherlayout *l,
			const t_lmodel *m)
{
	static const char	*vide[4] = {"", "Aucune appli r\xc3\xa9" "cente",
		"Aucune appli", "Aucun r\xc3\xa9sultat"};
	t_ltext				t;
	int32_t				i;

	if (m->nshown == 0)
	{
		t.font = FONT_UI;
		t.color = lp_pal()->muted;
		t.shadow = 0;
		t.text = vide[luna_lm_view(m)];
		t.box = luna_launcher_row_rect(l, 0);
		t.box.x += LL_ROW_PAD;
		t.align = LA_LEFT;
		lp_text(s, &t);
		return ;
	}
	i = 0;
	while (i < LUNA_LM_ROWS)
		row_one(s, l, m, i++);
	rows_bar(s, l, m);
}

void	luna_launcher_view(t_surface *s, const t_launcher *l,
			const t_lmodel *m, t_rect panel)
{
	t_launcherlayout	lay;
	t_lmview			v;

	if (m == NULL)
	{
		luna_launcher(s, l, panel);
		return ;
	}
	panel = lp_clean(panel);
	if (!lp_ok(s) || l == NULL || panel.w < LL_MIN_W || panel.h < LL_MIN_H)
		return ;
	luna_launcher_layout(panel, l->tiles == NULL ? 0 : l->ntiles, &lay);
	luna_glass(s, panel, LUNA_GLASS_LAUNCHER);
	lp_launch_query(s, &lay, m->query);
	v = luna_lm_view(m);
	lp_launch_tabs(s, &lay, (int32_t)v);
	if (v == LUNA_LV_PINNED)
		lp_launch_tiles(s, l, &lay);
	else
		lp_launch_rows(s, &lay, m);
	lp_launch_foot(s, l, &lay);
}
