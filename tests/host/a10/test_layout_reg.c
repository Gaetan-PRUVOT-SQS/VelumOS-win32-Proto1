#include "harness.h"
#include "th.h"

static void	registry(void)
{
	h_true(layout_find("fr") == &g_layout_fr, "fr trouvee");
	h_true(layout_find("us") == &g_layout_us, "us trouvee");
	h_true(layout_find("FR") == NULL, "casse differente");
	h_true(layout_find("") == NULL, "nom vide");
	h_true(layout_find("fr ") == NULL, "espace en fin");
	h_true(layout_find("frx") == NULL, "prefixe seul");
	h_true(layout_find("f") == NULL, "prefixe court");
	h_true(layout_find(NULL) == NULL, "pointeur nul");
	h_true(layout_default() == &g_layout_fr, "disposition par defaut fr");
	h_true(strlen(g_layout_fr.name) <= INPUT_LAYOUT_NAME_MAX, "nom fr borne");
	h_true(strlen(g_layout_us.name) <= INPUT_LAYOUT_NAME_MAX, "nom us borne");
}

static void	integrity(void)
{
	const t_layout	*l[2];
	uint32_t		i;
	uint32_t		k;

	l[0] = &g_layout_fr;
	l[1] = &g_layout_us;
	i = 0;
	while (i < 2)
	{
		h_eq_i64("codes de touche uniques", th_layout_dups(l[i]), 0);
		h_eq_i64("aucune collision avec les touches communes",
			th_layout_collisions(l[i]), 0);
		h_eq_u64("48 touches alphanumeriques", l[i]->nkeys, 48);
		k = 0;
		while (k < l[i]->nkeys)
		{
			h_true(l[i]->keys[k].vk != 0, "code virtuel non nul");
			h_true(l[i]->keys[k].ch[COL_NORMAL] != 0, "caractere de base");
			k++;
		}
		i++;
	}
}

static void	same_physical_keys(void)
{
	uint32_t	i;

	i = 0;
	while (i < g_layout_fr.nkeys)
	{
		h_true(layout_key(&g_layout_us, g_layout_fr.keys[i].code) != NULL,
			"touche fr aussi dans us");
		h_true(layout_key(&g_layout_fr, g_layout_us.keys[i].code) != NULL,
			"touche us aussi dans fr");
		i++;
	}
	h_eq_u64("AltGr : fr oui", g_layout_fr.altgr, 1);
	h_eq_u64("AltGr : us non", g_layout_us.altgr, 0);
}

int	main(void)
{
	h_begin("a10/layout_reg");
	h_run("registre des dispositions", registry);
	h_run("integrite des tables", integrity);
	h_run("memes touches physiques", same_physical_keys);
	return (h_end());
}
