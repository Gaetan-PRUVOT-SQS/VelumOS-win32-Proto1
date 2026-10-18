#include <string.h>
#include "harness.h"
#include "velum/luna_launcher.h"

static t_lmodel	g_m;

static void	garnir(void)
{
	static const t_lmapp	apps[] = {
		{"Bonjour", 0, ICON_PROGRAM},
		{"\xc3\x89" "diteur", 1, ICON_PROGRAM},
		{"Calculatrice", 2, ICON_PROGRAM},
		{"Console", 3, ICON_PROGRAM},
		{"Mon Navigateur", 4, ICON_PROGRAM},
		{"Zebre", 5, ICON_PROGRAM},
		{"Ex\xc3\xa9" "cuter...", 1000, ICON_RUN}
	};

	luna_lm_init(&g_m);
	h_true(luna_lm_set_apps(&g_m, apps, 7) == 0, "7 apps acceptees");
}

static void	scores(void)
{
	h_eq_i64("prefixe", luna_lm_match("Calculatrice", "cal"), 3);
	h_eq_i64("debut de mot", luna_lm_match("Mon Navigateur", "nav"), 2);
	h_eq_i64("sous-chaine", luna_lm_match("Calculatrice", "cul"), 1);
	h_eq_i64("absent", luna_lm_match("Calculatrice", "xyz"), 0);
	h_eq_i64("requete vide", luna_lm_match("Calculatrice", ""), 1);
	h_eq_i64("majuscules", luna_lm_match("calculatrice", "CAL"), 3);
	h_eq_i64("accent cote appli", luna_lm_match("\xc3\x89" "diteur", "edi"), 3);
	h_eq_i64("accent cote requete", luna_lm_match("Editeur", "\xc3\xa9" "di"), 3);
	h_eq_i64("requete plus longue", luna_lm_match("Ab", "abc"), 0);
	h_eq_i64("NULL", luna_lm_match(NULL, "a"), 0);
}

static void	recherche(void)
{
	garnir();
	h_true(luna_lm_type(&g_m, 'c') == 1, "saisie c");
	h_eq_i64("vue recherche", luna_lm_view(&g_m), LUNA_LV_SEARCH);
	h_eq_i64("c : Calculatrice, Console, Ex\xc3\xa9" "cuter", g_m.nshown, 3);
	h_eq_i64("prefixes d'abord", g_m.apps[g_m.shown[0]].id, 2);
	h_eq_i64("prefixe suivant", g_m.apps[g_m.shown[1]].id, 3);
	h_true(luna_lm_type(&g_m, 'o') == 1, "saisie o");
	h_eq_i64("co : Console seul", g_m.nshown, 1);
	h_eq_i64("selection initiale", g_m.sel, 0);
	h_true(luna_lm_backspace(&g_m) == 1, "effacement");
	h_eq_i64("retour a c", g_m.nshown, 3);
	h_true(luna_lm_backspace(&g_m) == 1, "effacement final");
	h_eq_i64("requete vide : vue epinglees", luna_lm_view(&g_m), LUNA_LV_PINNED);
	h_true(luna_lm_backspace(&g_m) == 0, "rien a effacer");
}

static void	utf8(void)
{
	garnir();
	h_true(luna_lm_type(&g_m, 0xe9) == 1, "saisie accent");
	h_eq_i64("deux octets", g_m.qlen, 2);
	h_eq_i64("e sans accent : tout sauf Bonjour et la liste", g_m.nshown, 6);
	h_true(luna_lm_backspace(&g_m) == 1, "effacement du caractere entier");
	h_eq_i64("requete vide", g_m.qlen, 0);
	h_true(luna_lm_type(&g_m, 0x20ac) == 1, "trois octets");
	h_eq_i64("euro : trois octets", g_m.qlen, 3);
	h_true(luna_lm_type(&g_m, 0x1f600) == 1, "quatre octets");
	h_eq_i64("sept octets", g_m.qlen, 7);
	luna_lm_clear_query(&g_m);
	h_eq_i64("clear", g_m.qlen, 0);
}

static void	refus(void)
{
	uint32_t	i;

	garnir();
	h_true(luna_lm_type(&g_m, 0x07) == 0, "controle refuse");
	h_true(luna_lm_type(&g_m, 0x7f) == 0, "DEL refuse");
	h_true(luna_lm_type(&g_m, 0x85) == 0, "C1 refuse");
	h_true(luna_lm_type(&g_m, 0xd800) == 0, "substitut refuse");
	h_true(luna_lm_type(&g_m, 0x110000) == 0, "hors plage refuse");
	h_true(luna_lm_type(NULL, 'a') == 0, "modele NULL");
	i = 0;
	while (i < LUNA_LM_QUERY_MAX + 8)
	{
		luna_lm_type(&g_m, 'a');
		i++;
	}
	h_eq_i64("requete bornee", g_m.qlen, LUNA_LM_QUERY_MAX - 1);
	h_eq_i64("terminateur", g_m.query[g_m.qlen], 0);
	h_true(luna_lm_set_apps(&g_m, NULL, LUNA_LM_APPS_MAX + 1) < 0, "trop d'apps");
}

static void	recents(void)
{
	garnir();
	luna_lm_set_tab(&g_m, LUNA_LV_RECENT);
	h_eq_i64("aucun recent", g_m.nshown, 0);
	luna_lm_touch(&g_m, 3);
	luna_lm_touch(&g_m, 1);
	luna_lm_touch(&g_m, 3);
	h_eq_i64("deduplique", g_m.nrecent, 2);
	h_eq_i64("plus recent d'abord", g_m.apps[g_m.shown[0]].id, 3);
	h_eq_i64("puis l'autre", g_m.apps[g_m.shown[1]].id, 1);
	luna_lm_touch(&g_m, 0);
	luna_lm_touch(&g_m, 2);
	luna_lm_touch(&g_m, 5);
	h_eq_i64("borne a 4", g_m.nrecent, LUNA_LM_RECENTS);
	h_eq_i64("tete", g_m.recent[0], 5);
	h_eq_i64("queue : le plus ancien est sorti", g_m.recent[3], 3);
	luna_lm_touch(&g_m, 4242);
	h_eq_i64("id inconnu ignore a l'affichage", g_m.nshown, 3);
}

static void	toutes(void)
{
	static const uint32_t	attendu[7] = {0, 2, 3, 1, 1000, 4, 5};
	uint32_t				i;

	garnir();
	luna_lm_set_tab(&g_m, LUNA_LV_ALL);
	h_eq_i64("toutes", g_m.nshown, 7);
	i = 0;
	while (i < 7)
	{
		h_eq_i64("ordre alphabetique sans accent", g_m.apps[g_m.shown[i]].id,
			attendu[i]);
		i++;
	}
	luna_lm_set_tab(&g_m, 99);
	h_eq_i64("onglet borne", g_m.tab, LUNA_LV_ALL);
	luna_lm_set_tab(&g_m, -5);
	h_eq_i64("onglet plancher", g_m.tab, LUNA_LV_PINNED);
}

static void	defilement(void)
{
	uint32_t	id;

	garnir();
	luna_lm_set_tab(&g_m, LUNA_LV_ALL);
	h_eq_i64("bas", luna_lm_move(&g_m, 1), 1);
	luna_lm_move(&g_m, 3);
	h_eq_i64("sel 4", g_m.sel, 4);
	h_eq_i64("haut de fenetre decale", g_m.top, 1);
	luna_lm_move(&g_m, 100);
	h_eq_i64("clamp bas", g_m.sel, 6);
	h_eq_i64("top max", g_m.top, 3);
	luna_lm_move(&g_m, -100);
	h_eq_i64("clamp haut", g_m.sel, 0);
	h_eq_i64("top revenu", g_m.top, 0);
	h_eq_i64("scroll borne", luna_lm_scroll(&g_m, 50), 3);
	h_eq_i64("scroll plancher", luna_lm_scroll(&g_m, -50), 0);
	h_true(luna_lm_selected(&g_m, &id) && id == 0, "selection : Bonjour");
	h_true(luna_lm_select(&g_m, 2) == 2, "clic ligne 2");
	h_true(luna_lm_select(&g_m, LUNA_LM_ROWS) < 0, "ligne hors plage");
	h_true(luna_lm_row(&g_m, 0) != NULL && luna_lm_row(&g_m, 9) == NULL,
		"lignes");
}

static void	vide(void)
{
	uint32_t	id;

	luna_lm_init(&g_m);
	luna_lm_set_tab(&g_m, LUNA_LV_ALL);
	h_eq_i64("rien a deplacer", luna_lm_move(&g_m, 1), LUNA_LM_NONE);
	h_true(luna_lm_selected(&g_m, &id) == 0, "pas de selection");
	h_true(luna_lm_row(&g_m, 0) == NULL, "pas de ligne");
	luna_lm_type(&g_m, 'z');
	h_eq_i64("aucun resultat", g_m.nshown, 0);
}

int	main(void)
{
	h_begin("a17/lanceur_recherche");
	h_run("scores de correspondance : partitions", scores);
	h_run("filtre et classement", recherche);
	h_run("saisie UTF-8", utf8);
	h_run("saisie refusee et bornes", refus);
	h_run("recents : MRU, doublons, borne", recents);
	h_run("toutes les apps : tri sans accent", toutes);
	h_run("defilement : valeurs limites", defilement);
	h_run("liste vide", vide);
	return (h_end());
}
