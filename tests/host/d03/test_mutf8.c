#include "harness.h"
#include "fake.h"
#include "dex_int.h"

static int	conv(const char *in, char *buf, size_t cap)
{
	t_text	t;

	t.p = buf;
	t.cap = cap;
	return (mutf8_to_utf8(in, t));
}

static void	convert(void)
{
	char	b[32];

	h_eq_i64("ascii", conv("abc", b, 32), 3);
	h_eq_str("ascii", b, "abc");
	h_eq_i64("2 et 3 octets", conv("\xc3\xa9\xe2\x82\xac", b, 32), 5);
	h_eq_str("inchange", b, "\xc3\xa9\xe2\x82\xac");
	h_eq_i64("paire", conv("\xed\xa0\xbd\xed\xb8\x80", b, 32), 4);
	h_eq_str("U+1F600", b, "\xf0\x9f\x98\x80");
	h_eq_i64("haut seul", conv("\xed\xa0\xbd", b, 32), E_INVAL);
	h_eq_i64("bas seul", conv("\xed\xb8\x80", b, 32), E_INVAL);
	h_eq_i64("U+0000", conv("a\xc0\x80", b, 32), E_NOTSUP);
	h_eq_i64("4 octets", conv("\xf0\x9f\x98\x80", b, 32), E_INVAL);
	h_eq_i64("continuation seule", conv("\x80", b, 32), E_INVAL);
	h_eq_i64("sur-longueur", conv("\xc1\x81", b, 32), E_INVAL);
	h_eq_i64("tronque", conv("\xe2\x82", b, 32), E_INVAL);
	h_eq_i64("juste", conv("abc", b, 4), 3);
	h_eq_i64("trop petit", conv("abc", b, 3), E_OVERFLOW);
	h_eq_i64("capacite nulle", conv("abc", b, 0), E_INVAL);
	h_eq_i64("vide", conv("", b, 1), 0);
}

static int	chk(const char *s, size_t n, uint32_t *units)
{
	t_span		sp;
	uint32_t	size;

	sp.p = (const uint8_t *)s;
	sp.len = n;
	*units = 0;
	return (dex_mutf8_check(sp, &size, units));
}

static void	check(void)
{
	uint32_t	u;

	h_eq_i64("termine", chk("ab", 3, &u), E_OK);
	h_eq_u64("unites", u, 2);
	h_eq_i64("non termine", chk("ab", 2, &u), E_INVAL);
	h_eq_i64("nul code", chk("\xc0\x80", 3, &u), E_OK);
	h_eq_u64("une unite", u, 1);
	h_eq_i64("sur-longueur 2", chk("\xc1\xbf", 3, &u), E_INVAL);
	h_eq_i64("sur-longueur 3", chk("\xe0\x9f\xbf", 4, &u), E_INVAL);
	h_eq_i64("3 octets mini", chk("\xe0\xa0\x80", 4, &u), E_OK);
	h_eq_i64("F0 interdit", chk("\xf0\x90\x80\x80", 5, &u), E_INVAL);
	h_eq_i64("FF interdit", chk("\xff", 2, &u), E_INVAL);
	h_eq_i64("continuation", chk("\x80", 2, &u), E_INVAL);
	h_eq_i64("coupe", chk("\xe2\x82", 2, &u), E_INVAL);
	h_eq_i64("substitut", chk("\xed\xa0\xbd", 4, &u), E_OK);
	h_eq_i64("vide", chk("", 0, &u), E_INVAL);
}

int	main(void)
{
	h_begin("d03/mutf8");
	h_run("MUTF-8 vers UTF-8", convert);
	h_run("validation MUTF-8", check);
	return (h_end());
}
