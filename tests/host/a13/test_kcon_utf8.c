#include "kfix.h"

static void	two_bytes(void)
{
	t_kfix	f;

	if (!kfix_open(&f, 80, 64, 80))
		return ;
	kfix_feed(&f, "\xc3\xa9t\xc3\xa9");
	h_eq_i64("2 octets : 3 cellules", f.k.col, 3);
	h_eq_u64("e aigu", kfix_glyph(2)->cp, 0xe9);
	h_true(kfix_is(&f, 1, 't', 1), "t en colonne 1");
	h_true(kfix_is(&f, 0, 0xe9, 2), "e aigu en colonne 2");
	kfix_close(&f);
}

static void	three_and_four_bytes(void)
{
	t_kfix	f;

	if (!kfix_open(&f, 80, 64, 80))
		return ;
	kfix_feed(&f, "\xe2\x82\xac");
	h_true(kfix_is(&f, 0, 0x20ac, 0), "euro sur une cellule");
	kfix_feed(&f, "\xf0\x9f\x98\x80");
	h_true(kfix_is(&f, 0, 0x1f600, 1), "plan supplementaire sur une cellule");
	h_eq_i64("deux cellules en tout", f.k.col, 2);
	kfix_close(&f);
}

static void	invalid_sequences(void)
{
	t_kfix	f;

	if (!kfix_open(&f, 80, 64, 80))
		return ;
	kfix_feed(&f, "\xff");
	h_true(kfix_is(&f, 0, 0xfffd, 0), "octet interdit : remplacement");
	kfix_feed(&f, "\xc0\x80");
	h_eq_i64("sur-longueur : cellules avancees", f.k.col >= 2, 1);
	kfix_feed(&f, "\xed\xa0\x80");
	h_eq_i64("substitut : cellules avancees", f.k.col >= 3, 1);
	kfix_feed(&f, "\xf4\x90\x80\x80");
	h_eq_i64("au-dela de 10FFFF : au plus 4 cellules", f.k.col <= 10, 1);
	kfix_close(&f);
}

static void	truncated_tail(void)
{
	t_kfix	f;

	if (!kfix_open(&f, 80, 64, 80))
		return ;
	kfix_feed(&f, "a\xe2\x82");
	h_eq_i64("fin tronquee : au moins une cellule", f.k.col >= 2, 1);
	h_eq_i64("fin tronquee : au plus deux", f.k.col <= 3, 1);
	h_true(kfix_is(&f, 0, 0xfffd, f.k.col - 1), "derniere cellule : U+FFFD");
	kfix_close(&f);
}

int	main(void)
{
	h_begin("a13/kcon_utf8");
	h_run("utf8 deux octets", two_bytes);
	h_run("utf8 trois et quatre octets", three_and_four_bytes);
	h_run("utf8 sequences invalides", invalid_sequences);
	h_run("utf8 fin tronquee", truncated_tail);
	return (h_end());
}
