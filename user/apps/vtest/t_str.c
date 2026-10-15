#include <limits.h>
#include "ctype.h"
#include "errno.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "vtest.h"

static int	cmp_int(const void *a, const void *b)
{
	int	x;
	int	y;

	x = *(const int *)a;
	y = *(const int *)b;
	return ((x > y) - (x < y));
}

static void	str_conv(void)
{
	char	*end;

	vtest_check(strtol("  -42xyz", &end, 10) == -42 && *end == 'x',
		"str: strtol signe et fin");
	vtest_check(strtol("0x1F", NULL, 0) == 31, "str: strtol base 0 hexa");
	vtest_check(strtoul("ff", NULL, 16) == 255, "str: strtoul base 16");
	errno = 0;
	vtest_check(strtol("99999999999999999999", NULL, 10) == LONG_MAX
		&& errno == ERANGE, "str: strtol depassement");
	vtest_check(atoi("123") == 123 && abs(-5) == 5, "str: atoi, abs");
}

static void	str_sort(void)
{
	static int	v[1000];
	int			i;
	int			ok;
	int			key;

	srand(7);
	i = -1;
	while (++i < 1000)
		v[i] = rand() % 5000;
	qsort(v, 1000, sizeof(v[0]), cmp_int);
	ok = 1;
	i = 0;
	while (++i < 1000)
		ok = ok && v[i - 1] <= v[i];
	vtest_check(ok, "str: qsort trie 1000 entiers");
	key = v[500];
	vtest_check(bsearch(&key, v, 1000, sizeof(v[0]), cmp_int) != NULL,
		"str: bsearch trouve un present");
	key = -1;
	vtest_check(bsearch(&key, v, 1000, sizeof(v[0]), cmp_int) == NULL,
		"str: bsearch rejette un absent");
}

static void	str_fmt(void)
{
	char	b[32];
	int		n;

	n = snprintf(b, sizeof(b), "%d|%5s|%-3c|%#x|%lld", -7, "ab", 'z', 255,
			(long long)INT64_MIN);
	vtest_check(strcmp(b, "-7|   ab|z  |0xff|-922337203685") == 0 && n == 38,
		"str: snprintf tronque et rend la longueur voulue");
	n = snprintf(b, sizeof(b), "%s=%u", "k", 42u);
	vtest_check(n == 4 && strcmp(b, "k=42") == 0, "str: snprintf simple");
	vtest_check(strcmp(strdup("abc"), "abc") == 0, "str: strdup");
}

void	vtest_str(void)
{
	int	c;
	int	digits;
	int	alphas;
	int	spaces;

	str_conv();
	str_sort();
	str_fmt();
	digits = 0;
	alphas = 0;
	spaces = 0;
	c = -1;
	while (++c < 256)
	{
		digits += isdigit(c) != 0;
		alphas += isalpha(c) != 0;
		spaces += isspace(c) != 0;
	}
	vtest_check(digits == 10 && alphas == 52 && spaces == 6, "str: ctype");
}
