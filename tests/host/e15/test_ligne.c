#include <string.h>
#include "harness.h"
#include "velum/err.h"
#include "e15.h"

static t_conline	g_l;

static void	typed(const char *s)
{
	line_insert(&g_l, s, (uint32_t)strlen(s));
}

static void	insertion_et_curseur(void)
{
	line_init(&g_l);
	typed("abc");
	line_left(&g_l);
	typed("X");
	h_eq_str("insertion au curseur", g_l.buf, "abXc");
	h_eq_u64("curseur apres insertion", g_l.cur, 3);
	line_home(&g_l);
	h_eq_u64("home", g_l.cur, 0);
	line_end(&g_l);
	h_eq_u64("fin", g_l.cur, 4);
}

static void	suppression_utf8(void)
{
	line_init(&g_l);
	typed("a\xc3\xa9" "b");
	line_left(&g_l);
	line_backspace(&g_l);
	h_eq_str("retour arriere retire e accent entier", g_l.buf, "ab");
	h_eq_u64("longueur octets", g_l.len, 2);
	line_home(&g_l);
	line_delete(&g_l);
	h_eq_str("suppr", g_l.buf, "b");
	line_backspace(&g_l);
	h_eq_str("retour arriere au debut sans effet", g_l.buf, "b");
	h_eq_u64("cellules", line_cells("\xc3\xa9\xe2\x80\xba" "x", 6), 3);
}

static void	edition_de_fin(void)
{
	line_init(&g_l);
	typed("echo salut");
	line_home(&g_l);
	line_right(&g_l);
	line_right(&g_l);
	line_right(&g_l);
	line_right(&g_l);
	line_kill_end(&g_l);
	h_eq_str("ctrl+k", g_l.buf, "echo");
	line_kill_all(&g_l);
	h_eq_str("ctrl+u", g_l.buf, "");
	h_eq_u64("curseur remis a zero", g_l.cur, 0);
}

static void	historique(void)
{
	char	out[CON_LINE_MAX];

	line_init(&g_l);
	typed("un");
	line_commit(&g_l, out);
	typed("deux");
	line_commit(&g_l, out);
	typed("deux");
	line_commit(&g_l, out);
	h_eq_u64("doublon consecutif ignore", g_l.hist_n, 2);
	typed("brouillon");
	line_hist_prev(&g_l);
	h_eq_str("haut : dernier", g_l.buf, "deux");
	line_hist_prev(&g_l);
	h_eq_str("haut : avant-dernier", g_l.buf, "un");
	line_hist_prev(&g_l);
	h_eq_str("haut au bout reste", g_l.buf, "un");
	line_hist_next(&g_l);
	line_hist_next(&g_l);
	h_eq_str("bas rend le brouillon", g_l.buf, "brouillon");
	line_kill_all(&g_l);
	line_commit(&g_l, out);
	h_eq_u64("ligne vide hors historique", g_l.hist_n, 2);
}

static void	historique_plein(void)
{
	char		out[CON_LINE_MAX];
	char		n[8];
	uint32_t	i;

	line_init(&g_l);
	i = 0;
	while (i < CON_HIST_MAX + 5)
	{
		n[0] = (char)('a' + i % 26);
		n[1] = (char)('a' + i / 26);
		n[2] = '\0';
		typed(n);
		line_commit(&g_l, out);
		i++;
	}
	h_eq_u64("historique borne", g_l.hist_n, CON_HIST_MAX);
	h_eq_str("le plus ancien est sorti", g_l.hist[0], "fa");
}

static void	debordement(void)
{
	uint32_t	i;
	int			rc;

	line_init(&g_l);
	rc = 0;
	i = 0;
	while (i < CON_LINE_MAX + 10 && rc == 0)
	{
		rc = line_insert(&g_l, "z", 1);
		i++;
	}
	h_eq_i64("ligne pleine refusee", rc, E_NOSPC);
	h_eq_u64("longueur bornee", g_l.len, CON_LINE_MAX - 1);
	h_eq_i64("insertion vide refusee", line_insert(&g_l, "", 0), E_NOSPC);
}

static void	encodage_utf8(void)
{
	char	b[4];

	h_eq_u64("ascii", utf8_put('a', b), 1);
	h_eq_u64("deux octets", utf8_put(0xe9, b), 2);
	h_eq_u64("trois octets", utf8_put(0x203a, b), 3);
	h_eq_u64("octets e2 80 ba", (uint8_t)b[0] << 16 | (uint8_t)b[1] << 8
		| (uint8_t)b[2], 0xe280ba);
	h_eq_u64("quatre octets", utf8_put(0x1f600, b), 4);
	h_eq_u64("substitut refuse", utf8_put(0xd800, b), 0);
	h_eq_u64("hors plage refuse", utf8_put(0x110000, b), 0);
}

int	main(void)
{
	h_begin("e15_ligne");
	h_run("insertion_et_curseur", insertion_et_curseur);
	h_run("suppression_utf8", suppression_utf8);
	h_run("edition_de_fin", edition_de_fin);
	h_run("historique", historique);
	h_run("historique_plein", historique_plein);
	h_run("debordement", debordement);
	h_run("encodage_utf8", encodage_utf8);
	return (h_end());
}
