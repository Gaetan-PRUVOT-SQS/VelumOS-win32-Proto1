#include <stdint.h>
#include "harness.h"
#include "velum/libk.h"
#include "a20_test.h"
#include "utf8.h"

static void	clean_ascii_controles_invalides(void)
{
	char	out[32];

	h_eq_u64("ascii", utf8_clean_copy(out, sizeof(out), "Bonjour", 7), 7);
	h_eq_str("ascii texte", out, "Bonjour");
	utf8_clean_copy(out, sizeof(out), "a\001b\177c\nd", 7);
	h_eq_str("controles remplaces", out, "a?b?c?d");
	utf8_clean_copy(out, sizeof(out), "a\303b\377", 4);
	h_eq_str("octets invalides remplaces", out, "a?b?");
	utf8_clean_copy(out, sizeof(out), "\303\251\342\202\254", 5);
	h_eq_str("sequences gardees", out, "\303\251\342\202\254");
}

static void	clean_limites_du_tampon(void)
{
	char	out[8];

	h_eq_u64("taille 0", utf8_clean_copy(out, 0, "abc", 3), 0);
	h_eq_u64("taille 1", utf8_clean_copy(out, 1, "abc", 3), 0);
	h_eq_str("taille 1 vide", out, "");
	h_eq_u64("taille 4", utf8_clean_copy(out, 4, "abcdef", 6), 3);
	h_eq_str("taille 4 texte", out, "abc");
	h_eq_u64("pas de sequence coupee",
		utf8_clean_copy(out, 5, "a\303\251\342\202\254", 6), 3);
	h_eq_str("avant la sequence de 3", out, "a\303\251");
	h_eq_u64("longueur nulle", utf8_clean_copy(out, 8, "abc", 0), 0);
	h_eq_str("longueur nulle vide", out, "");
}

int	main(void)
{
	h_begin("a20/utf8-clean");
	h_run("clean: ascii, controles", clean_ascii_controles_invalides);
	h_run("clean: limites du tampon", clean_limites_du_tampon);
	return (h_end());
}
