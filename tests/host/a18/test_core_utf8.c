#include <stdlib.h>
#include <string.h>
#include "harness.h"
#include "help.h"

static int	ok(const char *s, uint32_t n)
{
	uint8_t	*copy;
	int		r;

	copy = malloc(n + 1);
	memcpy(copy, s, n);
	r = wsp_utf8_ok(copy, n);
	free(copy);
	return (r);
}

static void	well_formed(void)
{
	h_true(ok("abc", 3), "ASCII");
	h_true(ok("", 0), "vide");
	h_true(ok("\xc2\xa0", 2), "U+00A0 premier non controle");
	h_true(ok("\xdf\xbf", 2), "U+07FF");
	h_true(ok("\xe0\xa0\x80", 3), "U+0800");
	h_true(ok("\xed\x9f\xbf", 3), "U+D7FF");
	h_true(ok("\xee\x80\x80", 3), "U+E000");
	h_true(ok("\xef\xbf\xbd", 3), "U+FFFD");
	h_true(ok("\xf0\x90\x80\x80", 4), "U+10000");
	h_true(ok("\xf4\x8f\xbf\xbf", 4), "U+10FFFF");
	h_true(ok("\xc5\x93\xe2\x80\xa6", 5), "oe et points de suspension");
}

static void	ill_formed(void)
{
	h_true(!ok("\x01", 1), "controle C0");
	h_true(!ok("\x7f", 1), "DEL");
	h_true(!ok("\xc2\x85", 2), "controle C1");
	h_true(!ok("\xc0\x80", 2), "sur-longueur C0");
	h_true(!ok("\xc1\xbf", 2), "sur-longueur C1");
	h_true(!ok("\xe0\x9f\xbf", 3), "sur-longueur 3 octets");
	h_true(!ok("\xed\xa0\x80", 3), "substitut");
	h_true(!ok("\xf0\x8f\xbf\xbf", 4), "sur-longueur 4 octets");
	h_true(!ok("\xf4\x90\x80\x80", 4), "> U+10FFFF");
	h_true(!ok("\xf5\x80\x80\x80", 4), "F5");
	h_true(!ok("\xe2\x82", 2), "tronque");
	h_true(!ok("\x80", 1), "suite isolee");
	h_true(!ok("\xe2\x28\xa1", 3), "suite invalide");
	h_true(!ok("\xff", 1), "FF");
}

static void	random_never_overreads(void)
{
	uint64_t	st;
	uint8_t		buf[16];
	int			i;
	uint32_t	n;
	uint32_t	k;

	st = hg_seed("a18/core_utf8");
	i = 0;
	while (i < 100000)
	{
		n = hg_below(&st, 16);
		k = 0;
		while (k < n)
			buf[k++] = (uint8_t)hg_next(&st);
		ok((const char *)buf, n);
		i++;
	}
	h_true(1, "100 000 suites aleatoires sans lecture hors tampon");
	h_true(!wsp_title_ok("abc\x01", 64), "titre avec controle");
	h_true(wsp_title_ok("", 64), "titre vide");
}

int	main(void)
{
	h_begin("a18/core_utf8");
	h_run("bien forme", well_formed);
	h_run("mal forme", ill_formed);
	h_run("aleatoire", random_never_overreads);
	return (h_end());
}
