#include <stdarg.h>
#include <stdint.h>
#include "harness.h"
#include "velum/klog.h"

static void	fmt_decimal(void)
{
	char	b[64];

	ksnprintf(b, sizeof(b), "%d|%i|%5d|%-5d|%05d", -42, 7, 42, 42, -42);
	h_eq_str("d", b, "-42|7|   42|42   |-0042");
	ksnprintf(b, sizeof(b), "%+d|% d|%u", 5, 5, 4000000000u);
	h_eq_str("signe", b, "+5| 5|4000000000");
	ksnprintf(b, sizeof(b), "%d|%3d|%.0d|%u", 0, 0, 0, 0u);
	h_eq_str("zero", b, "0|  0||0");
	ksnprintf(b, sizeof(b), "%lld|%llu", (long long)INT64_MIN, UINT64_MAX);
	h_eq_str("64 bits", b, "-9223372036854775808|18446744073709551615");
}

static void	fmt_hex(void)
{
	char	b[64];

	ksnprintf(b, sizeof(b), "%x|%X|%#x|%#llx|%08x", 255, 255, 255, 4096ull,
		48879);
	h_eq_str("hex", b, "ff|FF|0xff|0x1000|0000beef");
	ksnprintf(b, sizeof(b), "%#x|%o|%#o", 0, 8, 8);
	h_eq_str("zero et octal", b, "0|10|010");
	ksnprintf(b, sizeof(b), "%p", (void *)0x1234);
	h_eq_str("pointeur", b, "0x1234");
}

static void	fmt_strings(void)
{
	char	b[64];

	ksnprintf(b, sizeof(b), "%s|%10s|%-6s|%.3s|%c", "abc", "abc", "ab",
		"abcdef", 'z');
	h_eq_str("chaines", b, "abc|       abc|ab    |abc|z");
	ksnprintf(b, sizeof(b), "%s", (char *)0);
	h_eq_str("null", b, "(null)");
	ksnprintf(b, sizeof(b), "100%%|%q", 1);
	h_eq_str("pourcent et inconnu", b, "100%|%q");
}

static void	fmt_truncation(void)
{
	char	b[8];
	int		n;

	n = ksnprintf(b, sizeof(b), "%s", "0123456789");
	h_eq_i64("longueur voulue", n, 10);
	h_eq_str("tronque", b, "0123456");
	n = ksnprintf(b, 0, "abc");
	h_eq_i64("taille nulle", n, 3);
	n = ksnprintf(b, 1, "abc");
	h_eq_i64("taille un", n, 3);
	h_eq_str("taille un vide", b, "");
}

int	main(void)
{
	h_begin("skel/fmt");
	h_run("decimal", fmt_decimal);
	h_run("hexadecimal", fmt_hex);
	h_run("chaines", fmt_strings);
	h_run("troncature", fmt_truncation);
	return (h_end());
}
