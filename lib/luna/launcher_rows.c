#include "launcher_int.h"

static const int32_t	g_tab_x[3] = {0, 104, 202};
static const int32_t	g_tab_w[3] = {100, 94, 150};

t_rect	luna_launcher_tab_rect(const t_launcherlayout *l, int32_t tab)
{
	if (l == NULL || tab < 0 || tab > LUNA_LV_ALL)
		return (lp_rect(0, 0, 0, 0));
	return (lp_rect(l->title.x + g_tab_x[tab], l->title.y - LL_TAB_PAD,
			lp_min(g_tab_w[tab], lp_max(l->title.w - g_tab_x[tab], 0)),
			l->title.h + 2 * LL_TAB_PAD));
}

t_rect	luna_launcher_row_rect(const t_launcherlayout *l, int32_t row)
{
	int32_t	top;

	if (l == NULL || row < 0 || row >= LUNA_LM_ROWS)
		return (lp_rect(0, 0, 0, 0));
	top = l->title.y + LL_TITLE_H + LL_TITLE_GAP;
	return (lp_rect(l->search.x, top + row * LL_ROW_H,
			lp_max(l->search.w - LL_BAR_W - LL_FOCUS_GAP * 2, 0), LL_ROW_H));
}

int32_t	luna_launcher_tab_hit(const t_launcherlayout *l, int32_t x, int32_t y)
{
	int32_t	i;

	i = 0;
	while (l != NULL && i <= LUNA_LV_ALL)
	{
		if (lp_in(luna_launcher_tab_rect(l, i), lp_pt(x, y)))
			return (i);
		i++;
	}
	return (LUNA_LM_NONE);
}

int32_t	luna_launcher_row_hit(const t_launcherlayout *l, int32_t x, int32_t y)
{
	int32_t	i;

	i = 0;
	while (l != NULL && i < LUNA_LM_ROWS)
	{
		if (lp_in(luna_launcher_row_rect(l, i), lp_pt(x, y)))
			return (i);
		i++;
	}
	return (LUNA_LM_NONE);
}
