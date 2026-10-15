#include "luna_int.h"

t_rect	luna_window_client(const t_lunawin *w)
{
	t_lgeo	g;

	if (w == NULL)
		return (lp_rect(0, 0, 0, 0));
	lg_make(w, &g);
	return (g.client);
}

t_rect	luna_caption_rect(const t_lunawin *w)
{
	t_lgeo	g;

	if (w == NULL)
		return (lp_rect(0, 0, 0, 0));
	lg_make(w, &g);
	return (g.caption);
}

t_rect	luna_button_rect(const t_lunawin *w, uint32_t ht)
{
	t_lgeo	g;
	int32_t	i;

	if (w == NULL)
		return (lp_rect(0, 0, 0, 0));
	lg_make(w, &g);
	i = 0;
	while (i < LG_NBTN)
	{
		if (lg_btn_ht(i) == ht && (g.flags & (LG_SHOWN << i)))
			return (g.btn[i]);
		i++;
	}
	if (ht == HT_SYSMENU)
		return (g.sysmenu);
	return (lp_rect(g.outer.x, g.outer.y, 0, 0));
}
