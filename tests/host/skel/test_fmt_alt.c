#include <stdio.h>
#include "fmt_cases.h"
#include "harness.h"
#include "velum/klog.h"

static const t_altcase	g_alt[ALT_CASES] = {
{"%#o", 0, "0"}, {"%#.0o", 0, "0"}, {"%#.3o", 0, "000"},
{"%#o", 8, "010"}, {"%#.0o", 8, "010"}, {"%#.3o", 8, "010"},
{"%#x", 0, "0"}, {"%#.0x", 0, ""}, {"%#.3x", 0, "000"},
{"%#x", 255, "0xff"}, {"%#.0x", 255, "0xff"}, {"%#.3x", 255, "0x0ff"},
{"%o", 0, "0"}, {"%.0o", 0, ""}, {"%.3o", 0, "000"},
{"%o", 8, "10"}, {"%.0o", 8, "10"}, {"%.3o", 8, "010"},
{"%x", 0, "0"}, {"%.0x", 0, ""}, {"%.3x", 0, "000"},
{"%x", 255, "ff"}, {"%.0x", 255, "ff"}, {"%.3x", 255, "0ff"}
};

static void	alt_table(void)
{
	char	got[32];
	char	ref[32];
	int		i;

	i = 0;
	while (i < ALT_CASES)
	{
		ksnprintf(got, sizeof(got), g_alt[i].fmt, g_alt[i].val);
		snprintf(ref, sizeof(ref), g_alt[i].fmt, g_alt[i].val);
		h_eq_str(g_alt[i].fmt, got, g_alt[i].want);
		h_eq_str("oracle glibc", ref, g_alt[i].want);
		i++;
	}
}

static void	alt_voisins(void)
{
	char	b[64];

	ksnprintf(b, sizeof(b), "%#5o|%-#5o|%#05o|%#.2o", 0, 0, 0, 8);
	h_eq_str("largeur et # sur zero", b, "    0|0    |00000|010");
	ksnprintf(b, sizeof(b), "%#X|%#.0X|%#llo|%#o", 255, 0, 0ull, 0100);
	h_eq_str("X, long long, chiffre de tete non nul", b, "0XFF||0|0100");
}

int	main(void)
{
	h_begin("skel/fmt_alt");
	h_run("I-F3-2 table # x base x valeur x precision", alt_table);
	h_run("I-F3-2 voisins largeur et drapeaux", alt_voisins);
	return (h_end());
}
