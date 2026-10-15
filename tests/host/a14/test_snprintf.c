#include <limits.h>
#include <stdint.h>
#include "fake_f3.h"
#include "harness.h"
#include "stdio.h"

static void	fmt_decimal(void)
{
	char	b[96];

	snprintf(b, sizeof(b), "%d|%i|%d|%d", 0, -42, INT_MAX, INT_MIN);
	h_eq_str("d, i, bornes int", b, "0|-42|2147483647|-2147483648");
	snprintf(b, sizeof(b), "%5d|%-5d|%05d|%+d|% d", 42, 42, -42, 5, 5);
	h_eq_str("largeur, gauche, zeros, plus, espace", b,
		"   42|42   |-0042|+5| 5");
	snprintf(b, sizeof(b), "%.3d|%5.3d|%.0d|%.0d", 7, 7, 0, 5);
	h_eq_str("precision", b, "007|  007||5");
	snprintf(b, sizeof(b), "%+d|% d|%+d", 0, 0, -3);
	h_eq_str("signe de zero", b, "+0| 0|-3");
}

static void	fmt_unsigned(void)
{
	char	b[96];

	snprintf(b, sizeof(b), "%u|%u|%x", 0u, UINT_MAX, 0xffffffffu);
	h_eq_str("u, x, bornes", b, "0|4294967295|ffffffff");
	snprintf(b, sizeof(b), "%x|%X|%#x|%#X|%#x", 255, 255, 255, 255, 0);
	h_eq_str("hexadecimal et prefixe", b, "ff|FF|0xff|0XFF|0");
	snprintf(b, sizeof(b), "%o|%#o|%08x|%-8x|", 8, 8, 48879, 48879);
	h_eq_str("octal, zeros, gauche", b, "10|010|0000beef|beef    |");
	snprintf(b, sizeof(b), "%#o|%#6x|%#06x", 0, 255, 255);
	h_eq_str("o alterne de 0 : un seul zero", b,
		"0|  0xff|0x00ff");
}

static void	fmt_lengths(void)
{
	char	b[192];

	snprintf(b, sizeof(b), "%ld|%lld|%zu|%lu", LONG_MIN, LLONG_MIN, SIZE_MAX,
		ULONG_MAX);
	h_eq_str("l, ll, z", b,
		"-9223372036854775808|-9223372036854775808|18446744073709551615|"
		"18446744073709551615");
	snprintf(b, sizeof(b), "%hd|%hhd|%hu|%hhu|%llu", 70000, 300, 70000, 300,
		ULLONG_MAX);
	h_eq_str("h, hh, ull", b, "4464|44|4464|44|18446744073709551615");
	snprintf(b, sizeof(b), "%jd|%p|%p", (intmax_t)INT64_MIN, (void *)0x1234,
		(void *)0xdeadbeef);
	h_eq_str("j, pointeurs", b, "-9223372036854775808|0x1234|0xdeadbeef");
}

static void	fmt_flags_width(void)
{
	char	b[96];

	snprintf(b, sizeof(b), "%*d|%-*d|%*d", 6, 42, 6, 42, -6, 42);
	h_eq_str("largeur par argument", b, "    42|42    |42    ");
	snprintf(b, sizeof(b), "%.*d|%0*d|%.*d", 5, 42, 5, 42, -5, 7);
	h_eq_str("precision et zeros par argument", b, "00042|00042|7");
	f3_fmt(b, sizeof(b), "%-05d|%+05d|%05.3d|% +d|%+u", 42, 42, 42, 5, 5u);
	h_eq_str("combinaisons de drapeaux", b, "42   |+0042|  042|+5|5");
}

int	main(void)
{
	h_begin("a14/snprintf");
	h_run("snprintf/partition : entiers signes", fmt_decimal);
	h_run("snprintf/partition : entiers non signes", fmt_unsigned);
	h_run("snprintf/partition : modificateurs de longueur", fmt_lengths);
	h_run("snprintf/table de decision : drapeaux et largeurs", fmt_flags_width);
	return (h_end());
}
