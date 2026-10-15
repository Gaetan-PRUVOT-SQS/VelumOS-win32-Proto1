#include <stdint.h>
#include "fake_f3.h"
#include "harness.h"
#include "stdio.h"

static void	str_basic(void)
{
	char		b[96];
	const char	*nul;

	nul = NULL;
	snprintf(b, sizeof(b), "%s|%10s|%-6s|%.3s|%c|%5c|%-3c|", "abc", "abc", "ab",
		"abcdef", 'z', 'y', 'x');
	h_eq_str("chaines et caracteres", b,
		"abc|       abc|ab    |abc|z|    y|x  |");
	snprintf(b, sizeof(b), "%.0s|%.10s|%s|100%%", "abc", "abc", "");
	h_eq_str("precision de chaine, vide, pourcent", b, "|abc||100%");
	f3_fmt(b, sizeof(b), "%s|%10s", nul, nul);
	h_eq_str("pointeur nul", b, "(null)|    (null)");
	snprintf(b, sizeof(b), "%.*s|%-*.*s|", 2, "abcdef", 5, 3, "abcdef");
	h_eq_str("precision par argument", b, "ab|abc  |");
}

static void	str_trunc(void)
{
	char	b[8];
	int		n;

	n = snprintf(b, sizeof(b), "%s", "0123456789");
	h_eq_i64("longueur voulue", n, 10);
	h_eq_str("tronque", b, "0123456");
	n = snprintf(b, sizeof(b), "%s", "0123456");
	h_eq_i64("tient exactement", n, 7);
	h_eq_str("tient exactement : contenu", b, "0123456");
	n = snprintf(b, sizeof(b), "%s", "01234567");
	h_eq_i64("un de trop", n, 8);
	h_eq_str("un de trop : contenu", b, "0123456");
	n = snprintf(b, 4, "ab%dX", 12345);
	h_eq_i64("coupe dans une conversion", n, 8);
	h_eq_str("coupe dans une conversion : contenu", b, "ab1");
}

static void	str_sizes(void)
{
	char	b[8];
	int		n;

	b[0] = 'Z';
	b[1] = 'Z';
	b[2] = 'Z';
	b[3] = 'Z';
	n = snprintf(b, 0, "abc");
	h_eq_i64("taille 0", n, 3);
	h_true(b[0] == 'Z', "taille 0 : tampon intact");
	h_eq_i64("buf NULL, taille 0", snprintf(NULL, 0, "%d", 12345), 5);
	n = snprintf(b, 1, "abc");
	h_eq_i64("taille 1", n, 3);
	h_true(b[0] == '\0' && b[1] == 'Z', "taille 1 : terminateur seul");
	n = snprintf(b, 2, "abc");
	h_true(b[0] == 'a' && b[1] == '\0' && b[2] == 'Z', "taille 2");
	n = snprintf(b, 4, "abc");
	h_eq_i64("taille 4, tient", n, 3);
	h_eq_str("taille 4, contenu", b, "abc");
}

int	main(void)
{
	h_begin("a14/snprintf_str");
	h_run("snprintf/partition : chaines, caracteres, precision", str_basic);
	h_run("snprintf/limite : troncature", str_trunc);
	h_run("snprintf/limite : tailles 0, 1, 2, 4", str_sizes);
	return (h_end());
}
