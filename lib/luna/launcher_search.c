#include "launcher_int.h"

void	lp_launch_search(t_surface *s, const t_launcherlayout *l)
{
	t_cpal	p;
	t_ltext	t;
	t_rect	r;

	p = lp_pal();
	r = l->search;
	lp_launch_fill(s, r, r.h / 2, p->input);
	lp_fill(s, lp_rect(r.x + r.h / 2, r.y, r.w - r.h, 1),
		lp_launch_tone(LL_INPUT_RIM_L, LL_INPUT_RIM_D));
	luna_icon(s, lp_pt(r.x + LL_SEARCH_PAD,
			r.y + (r.h - LL_SEARCH_ICON) / 2),
		ICON_SEARCH, LL_SEARCH_ICON);
	t.font = FONT_UI;
	t.color = p->muted;
	t.shadow = 0;
	t.text = "Rechercher une app, un fichier, un r\xc3\xa9glage";
	t.box = lp_rect(r.x + LL_SEARCH_PAD + LL_SEARCH_ICON + LL_SEARCH_PAD, r.y,
			r.w - 2 * LL_SEARCH_PAD - LL_SEARCH_ICON - LL_SEARCH_PAD_R, r.h);
	t.align = LA_LEFT;
	lp_text(s, &t);
}

void	lp_launch_title(t_surface *s, const t_launcherlayout *l)
{
	t_ltext	t;

	t.font = FONT_UI_BOLD;
	t.color = lp_pal()->ink;
	t.shadow = 0;
	t.text = "\xc3\x89pingl\xc3\xa9" "es";
	t.box = lp_rect(l->title.x + LL_TITLE_PAD, l->title.y,
			l->title.w - 2 * LL_TITLE_PAD, l->title.h);
	t.align = LA_LEFT;
	lp_text(s, &t);
}
