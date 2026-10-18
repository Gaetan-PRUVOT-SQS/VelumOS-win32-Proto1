#include <string.h>
#include "harness.h"
#include "e15.h"

static t_console	g_c;
static t_conlive	g_live;

static void	insere(const char *s)
{
	line_insert(&g_c.line, s, (uint32_t)strlen(s));
}

static void	ligne_courte(void)
{
	con_init(&g_c, &g_fake_ops, NULL, 100);
	insere("ls");
	con_live_row(&g_c, 100, &g_live);
	h_eq_str("invite et saisie", g_live.text, "utilisateur@velum / > ls");
	h_eq_u64("curseur apres la saisie", g_live.cursor, 24);
	h_eq_u64("cellules", g_live.cells, 24);
	h_eq_u64("couleur utilisateur", g_live.color[0], CC_USER);
	h_eq_u64("couleur saisie", g_live.color[22], CC_TEXT);
	line_left(&g_c.line);
	con_live_row(&g_c, 100, &g_live);
	h_eq_u64("curseur sur le dernier caractere", g_live.cursor, 23);
}

static void	defilement_horizontal(void)
{
	con_init(&g_c, &g_fake_ops, NULL, 30);
	insere("0123456789012345678901234567890123456789");
	con_live_row(&g_c, 30, &g_live);
	h_eq_u64("curseur au bord droit", g_live.cursor, 29);
	h_eq_u64("29 caracteres et la cellule du curseur", g_live.cells, 29);
	h_eq_u64("queue de la saisie visible", (uint8_t)g_live.text[28], '9');
	line_home(&g_c.line);
	con_live_row(&g_c, 30, &g_live);
	h_eq_u64("curseur au debut de la saisie", g_live.cursor, 22);
	h_eq_u64("invite visible", (uint8_t)g_live.text[0], 'u');
}

static void	saisie_utf8(void)
{
	con_init(&g_c, &g_fake_ops, NULL, 100);
	insere("\xc3\xa9t\xc3\xa9");
	con_live_row(&g_c, 100, &g_live);
	h_eq_u64("curseur en cellules", g_live.cursor, 25);
	h_eq_u64("octets", g_live.bytes, 25 + 2);
}

static void	ligne_maximale(void)
{
	uint32_t	i;

	con_init(&g_c, &g_fake_ops, NULL, 160);
	i = 0;
	while (i < CON_LINE_MAX)
	{
		line_insert(&g_c.line, "\xe2\x80\xba", 3);
		i++;
	}
	con_live_row(&g_c, 160, &g_live);
	h_true(g_live.bytes < CON_LIVE_BYTES, "ligne vivante bornee");
	h_true(g_live.cursor < 160, "curseur dans la fenetre");
}

int	main(void)
{
	h_begin("e15_vue");
	h_run("ligne_courte", ligne_courte);
	h_run("defilement_horizontal", defilement_horizontal);
	h_run("saisie_utf8", saisie_utf8);
	h_run("ligne_maximale", ligne_maximale);
	return (h_end());
}
