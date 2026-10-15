#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include "ctype.h"
#include "fake_f3.h"
#include "harness.h"

#define CT_N 12
#define CT_UP "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
#define CT_LOW "abcdefghijklmnopqrstuvwxyz"
#define CT_DIGIT "0123456789"
#define CT_PUNCT "!\"#$%&'()*+,-./:;<=>?@[\\]^_`{|}~"

static const t_f3_ctype	g_ct[CT_N] = {
{"isalnum", isalnum, CT_DIGIT CT_UP CT_LOW, 1, 0, -1},
{"isalpha", isalpha, CT_UP CT_LOW, 1, 0, -1},
{"isblank", isblank, " \t", 1, 0, -1},
{"iscntrl", iscntrl, NULL, 0, 31, 127},
{"isdigit", isdigit, CT_DIGIT, 1, 0, -1},
{"isgraph", isgraph, NULL, 33, 126, -1},
{"islower", islower, CT_LOW, 1, 0, -1},
{"isprint", isprint, NULL, 32, 126, -1},
{"ispunct", ispunct, CT_PUNCT, 1, 0, -1},
{"isspace", isspace, " \t\n\v\f\r", 1, 0, -1},
{"isupper", isupper, CT_UP, 1, 0, -1},
{"isxdigit", isxdigit, CT_DIGIT "abcdefABCDEF", 1, 0, -1},
};

static int	ct_count_bad(const t_f3_ctype *d, const uint8_t *exp)
{
	int	c;
	int	bad;

	bad = 0;
	c = 0;
	while (c < 256)
	{
		if ((d->fn(c) != 0) != exp[c])
		{
			fprintf(stderr, "  %s(%d) : attendu %d\n", d->name, c, exp[c]);
			bad++;
		}
		c++;
	}
	return (bad);
}

static void	ct_classes(void)
{
	uint8_t	exp[256];
	int		i;

	i = 0;
	while (i < CT_N)
	{
		f3_ctype_build(&g_ct[i], exp);
		h_eq_i64(g_ct[i].name, ct_count_bad(&g_ct[i], exp), 0);
		i++;
	}
}

static void	ct_case_maps(void)
{
	int	c;
	int	low;
	int	up;
	int	bad;

	bad = 0;
	c = 0;
	while (c < 256)
	{
		low = c;
		up = c;
		if (c >= 'A' && c <= 'Z')
			low = c + 32;
		if (c >= 'a' && c <= 'z')
			up = c - 32;
		bad += tolower(c) != low;
		bad += toupper(c) != up;
		c++;
	}
	h_eq_i64("tolower, toupper sur 0..255", bad, 0);
}

static void	ct_outside(void)
{
	static const int	vals[7] = {-1, 256, -2, INT_MIN, INT_MAX, 1000, -128};
	int					i;
	int					j;
	int					bad;

	i = 0;
	bad = 0;
	while (i < 7)
	{
		j = 0;
		while (j < CT_N)
			bad += g_ct[j++].fn(vals[i]) != 0;
		bad += tolower(vals[i]) != vals[i];
		bad += toupper(vals[i]) != vals[i];
		i++;
	}
	h_eq_i64("EOF, 256, negatifs, INT_MIN, INT_MAX", bad, 0);
	h_eq_i64("EOF n'est pas une lettre", isalpha(EOF), 0);
}

int	main(void)
{
	h_begin("a14/ctype");
	h_run("ctype/partition : 12 classes sur 0..255", ct_classes);
	h_run("tolower, toupper/partition : 0..255", ct_case_maps);
	h_run("ctype/limite : hors de la plage", ct_outside);
	return (h_end());
}
