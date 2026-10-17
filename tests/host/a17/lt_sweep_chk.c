#include <stdio.h>
#include "harness.h"
#include "lt_hit.h"

void	lt_sweep_report(const t_lunawin *w, const t_ltsweep *s)
{
	char	msg[128];

	snprintf(msg, sizeof(msg), "%dx%d style 0x%04x max %d : %u violation(s) a "
		"(%d,%d)", w->outer.w, w->outer.h, w->style, w->maximized, s->bad,
		s->bad_x, s->bad_y);
	h_true(s->bad == 0, msg);
}

void	lt_sweep_counts(const t_lunawin *w, const t_ltsweep *s)
{
	uint32_t	total;
	int			i;

	total = 0;
	i = 0;
	while (i < 21)
		total += s->count[i++];
	h_true(total == lt_area(w->outer), "chaque pixel a exactement une valeur");
	h_true(s->count[HT_CLIENT] + s->edge_client
		== lt_area(luna_window_client(w)), "client, bandes de bord comprises");
	h_true(s->count[HT_CLOSE] + s->edge_close
		== lt_area(luna_button_rect(w, HT_CLOSE)),
		"pixels du bouton fermer, bandes de bord comprises");
	h_true(s->count[HT_SYSMENU] <= lt_area(luna_button_rect(w, HT_SYSMENU)),
		"pixels du menu systeme");
	h_true(s->count[HT_SYSMENU] + s->edge_sys
		== lt_area(luna_button_rect(w, HT_SYSMENU)),
		"menu systeme, bandes de bord comprises");
}

void	lt_same_zones(const t_lunawin *a, const t_lunawin *b, const char *msg)
{
	t_point	p;

	p.y = 0;
	while (p.y < a->outer.h)
	{
		p.x = 0;
		while (p.x < a->outer.w)
		{
			h_true(luna_hit_test(a, p) == luna_hit_test(b, p), msg);
			p.x++;
		}
		p.y++;
	}
}
