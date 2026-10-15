#include <stdio.h>
#include "harness.h"
#include "lt_hit.h"

static void	meta_translate_one(t_point d, uint32_t style)
{
	t_lunawin	a;
	t_lunawin	b;
	t_point		p;
	char		msg[96];

	a = lt_win(lp_rect(5, 9, 220, 140), style, false);
	b = lt_win(lp_rect(5 + d.x, 9 + d.y, 220, 140), style, false);
	p.y = 0;
	while (p.y < 140)
	{
		p.x = 0;
		while (p.x < 220)
		{
			snprintf(msg, sizeof(msg), "style 0x%x decalage (%d,%d) pixel "
				"(%d,%d)", style, d.x, d.y, p.x, p.y);
			h_true(luna_hit_test(&a, lp_pt(5 + p.x, 9 + p.y))
				== luna_hit_test(&b, lp_pt(5 + d.x + p.x, 9 + d.y + p.y)), msg);
			p.x += 3;
		}
		p.y += 3;
	}
}

static void	meta_translation(void)
{
	static const t_point	offsets[4] = {{-500, -300}, {123456, -98765},
	{-100000000, 100000000}, {1, 1}};
	int						i;

	i = 0;
	while (i < 4)
	{
		meta_translate_one(offsets[i], WS_DEFAULT);
		meta_translate_one(offsets[i], 0x0f);
		meta_translate_one(offsets[i], 0x43);
		i++;
	}
}

static void	meta_irrelevant_bits(void)
{
	t_lunawin	a;
	t_lunawin	b;
	uint32_t	extra;
	uint32_t	bits;

	extra = 0;
	while (extra < 8)
	{
		bits = (extra & 1) * 0x20 + ((extra >> 1) & 1) * 0x80
			+ ((extra >> 2) & 1) * 0x100;
		a = lt_win(lp_rect(0, 0, 130, 70), WS_DEFAULT, false);
		b = lt_win(lp_rect(0, 0, 130, 70), WS_DEFAULT | bits, false);
		lt_same_zones(&a, &b, "POPUP, NOACTIVATE et TOPMOST sans effet");
		extra++;
	}
}

static void	meta_state_fields(void)
{
	t_lunawin	a;
	t_lunawin	b;

	a = lt_win(lp_rect(0, 0, 130, 70), WS_DEFAULT, false);
	b = a;
	b.title = NULL;
	b.icon = ICON_FOLDER;
	b.active = false;
	b.hot = HT_CLOSE;
	b.pressed = HT_MINBUTTON;
	lt_same_zones(&a, &b, "titre, icone, activite, survol sans effet");
}

int	main(void)
{
	h_begin("a17/hit_meta");
	h_run("invariance par translation", meta_translation);
	h_run("bits de style sans effet", meta_irrelevant_bits);
	h_run("champs d'etat sans effet", meta_state_fields);
	return (h_end());
}
