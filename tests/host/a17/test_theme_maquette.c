#include <stdio.h>
#include <string.h>
#include "harness.h"
#include "lt_theme_parse.h"

static void	maquette_header(void)
{
	FILE			*f;
	char			line[256];
	char			name[64];
	uint32_t		v;
	const t_lthv	*e;
	size_t			seen;

	f = fopen(LTH_MAQUETTE, "r");
	h_true(f != NULL, "la maquette velum_theme.h est lisible");
	if (f == NULL)
		return ;
	seen = 0;
	while (fgets(line, sizeof(line), f) != NULL)
	{
		if (!lth_parse_line(line, name, sizeof(name), &v))
			continue ;
		e = lth_find(name);
		h_true(e != NULL, name);
		if (e != NULL)
			h_eq_u64(name, e->value, v);
		seen++;
	}
	fclose(f);
	h_eq_i64("chaque jeton numerique de la maquette est dans theme.h",
		(int64_t)seen, (int64_t)lth_count());
}

static void	readme_values(void)
{
	size_t	i;

	i = 0;
	while (g_lth_pct[i].name != NULL)
	{
		h_eq_u64(g_lth_pct[i].name, g_lth_pct[i].value,
			lth_pct(g_lth_pct[i].pct, g_lth_pct[i].rgb));
		i++;
	}
	i = 0;
	while (g_lth_hex[i].name != NULL)
	{
		h_eq_u64(g_lth_hex[i].name, g_lth_hex[i].value,
			lth_hex(g_lth_hex[i].hex));
		i++;
	}
}

int	main(void)
{
	h_begin("a17/theme_maquette");
	h_run("P9 theme.h recalcule depuis velum_theme.h (analyse du fichier)",
		maquette_header);
	h_run("P9 README : pourcentages, teintes et couleurs hexadecimales",
		readme_values);
	return (h_end());
}
