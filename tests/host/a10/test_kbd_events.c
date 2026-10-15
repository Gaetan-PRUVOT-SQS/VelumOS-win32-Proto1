#include "harness.h"
#include "th.h"

static const t_kcase	g_cases[] = {
{"a (touche q)", {0}, TH_LN, 0x15, 2, 'A', TH_N, 'a', TH_N},
{"Maj gauche + a", {0x12}, TH_LN, 0x15, 2, 'A', TH_S | TH_N, 'A', TH_S | TH_N},
{"Maj droite + a", {0x59}, TH_LN, 0x15, 2, 'A', TH_S | TH_N, 'A', TH_S | TH_N},
{"& sans Maj", {0}, TH_LN, 0x16, 2, '1', TH_N, '&', TH_N},
{"1 avec Maj", {0x12}, TH_LN, 0x16, 2, '1', TH_S | TH_N, '1', TH_S | TH_N},
{"AltGr + 0 = @", {0xe011}, TH_LN, 0x45, 2, '0', TH_A | TH_N, '@', TH_N},
{"Ctrl+Alt + 0 = @", {0x14, 0x11}, TH_LN, 0x45, 2, '0', TH_C | TH_A | TH_N,
	'@', TH_N},
{"Maj + AltGr + 0 : rien", {0x12, 0xe011}, TH_LN, 0x45, 1, '0',
	TH_S | TH_A | TH_N, 0, 0},
{"Ctrl + a = 0x01", {0x14}, TH_LN, 0x15, 2, 'A', TH_C | TH_N, 1,
	TH_C | TH_N},
{"Ctrl + 1 : rien", {0x14}, TH_LN, 0x16, 1, '1', TH_C | TH_N, 0, 0},
{"Ctrl + Entree", {0x14}, TH_LN, 0x5a, 2, 0x0d, TH_C | TH_N, 0x0d,
	TH_C | TH_N},
{"Alt + a : pas de CHAR", {0x11}, TH_LN, 0x15, 1, 'A', TH_A | TH_N, 0, 0},
{"Win + a : pas de CHAR", {0xe01f}, TH_LN, 0x15, 1, 'A', TH_W | TH_N, 0, 0},
{"Entree", {0}, TH_LN, 0x5a, 2, 0x0d, TH_N, 0x0d, TH_N},
{"Entree du pave", {0}, TH_LN, 0xe05a, 2, 0x0d, TH_N | TH_X, 0x0d, TH_N},
{"Espace", {0}, TH_LN, 0x29, 2, 0x20, TH_N, ' ', TH_N},
{"Tabulation", {0}, TH_LN, 0x0d, 2, 0x09, TH_N, 0x09, TH_N},
{"Retour arriere", {0}, TH_LN, 0x66, 2, 0x08, TH_N, 0x08, TH_N},
{"Echap", {0}, TH_LN, 0x76, 2, 0x1b, TH_N, 0x1b, TH_N},
{"F1 sans CHAR", {0}, TH_LN, 0x05, 1, 0x70, TH_N, 0, 0},
{"Fleche haut etendue", {0}, TH_LN, 0xe075, 1, 0x26, TH_N | TH_X, 0, 0},
{"Maj gauche seule", {0}, TH_LN, 0x12, 1, 0x10, TH_S | TH_N, 0, 0},
{"Ctrl droit etendu", {0}, TH_LN, 0xe014, 1, 0x11, TH_C | TH_X | TH_N, 0, 0},
{"Alt droit etendu", {0}, TH_LN, 0xe011, 1, 0x12, TH_A | TH_X | TH_N, 0, 0},
{"Win gauche", {0}, TH_LN, 0xe01f, 1, 0x5b, TH_W | TH_X | TH_N, 0, 0},
{"Menu contextuel", {0}, TH_LN, 0xe02f, 1, 0x5d, TH_X | TH_N, 0, 0},
{"Impr ecran", {0}, TH_LN, 0xe07c, 1, 0x2c, TH_X | TH_N, 0, 0},
{"Pause sans drapeau etendu", {0}, TH_LN, 0xe114, 1, 0x13, TH_N, 0, 0},
{"pave 5 avec Verr Num", {0}, TH_LN, 0x73, 2, 0x65, TH_N, '5', TH_N},
{"pave 5 sans Verr Num", {0}, 0, 0x73, 1, 0x0c, 0, 0, 0},
{"pave 8 Maj + Verr Num : fleche", {0x12}, TH_LN, 0x75, 1, 0x26, TH_S | TH_N,
	0, 0},
{"pave 8 Maj sans Verr Num : chiffre", {0x12}, 0, 0x75, 2, 0x68, TH_S, '8',
	TH_S},
{"pave plus", {0}, TH_LN, 0x79, 2, 0x6b, TH_N, '+', TH_N},
{"pave diviser", {0}, TH_LN, 0xe04a, 2, 0x6f, TH_N | TH_X, '/', TH_N},
{"Verr Maj + a", {0}, TH_LN | TH_LC, 0x15, 2, 'A', TH_N | TH_CAPS, 'A',
	TH_N | TH_CAPS},
{"Verr Maj + e accent", {0}, TH_LC, 0x1e, 2, '2', TH_CAPS, 0xc9, TH_CAPS},
{"touche multimedia inconnue", {0}, TH_LN, 0xe021, 0, 0, 0, 0, 0},
};

static void	run_table(void)
{
	uint32_t	i;

	i = 0;
	while (i < sizeof(g_cases) / sizeof(g_cases[0]))
	{
		th_run_kcase(&g_layout_fr, &g_cases[i]);
		i++;
	}
}

int	main(void)
{
	h_begin("a10/kbd_events");
	h_run("table de decision clavier fr", run_table);
	return (h_end());
}
