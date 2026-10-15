#include <stdio.h>
#include "harness.h"
#include "lt_theme_parse.h"
#include "luna_int.h"

static void	palette_theme(const t_lthf *tab, t_lunatheme th, const char *tag)
{
	t_color	got;
	char	name[64];
	size_t	i;

	luna_theme_set(th);
	i = 0;
	while (tab[i].name != NULL)
	{
		got = *(const t_color *)((const uint8_t *)lp_pal() + tab[i].off);
		snprintf(name, sizeof(name), "palette %s : %s", tag, tab[i].name);
		h_eq_u64(name, got, lth_pct(tab[i].pct, tab[i].rgb));
		i++;
	}
	luna_theme_set(LUNA_THEME_LIGHT);
}

static void	pal_light(void)
{
	palette_theme(g_lth_light, LUNA_THEME_LIGHT, "clair");
}

static void	pal_dark(void)
{
	palette_theme(g_lth_dark, LUNA_THEME_DARK, "sombre");
}

int	main(void)
{
	h_begin("a17/theme_pal");
	h_run("P9 palette clair recalculee depuis le README", pal_light);
	h_run("P9 palette sombre recalculee depuis le README", pal_dark);
	return (h_end());
}
