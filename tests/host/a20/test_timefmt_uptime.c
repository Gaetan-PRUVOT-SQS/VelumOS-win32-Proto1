#include <stdint.h>
#include "harness.h"
#include "velum/libk.h"
#include "a20_test.h"
#include "timefmt.h"
#include "typo.h"

static const t_clockrow	g_rows[] = {
{0, "0" NBSP "s"},
{999999999ull, "0" NBSP "s"},
{59 * NS_SEC, "59" NBSP "s"},
{60 * NS_SEC, "1" NBSP "min 00" NBSP "s"},
{61 * NS_SEC, "1" NBSP "min 01" NBSP "s"},
{3599 * NS_SEC, "59" NBSP "min 59" NBSP "s"},
{3600 * NS_SEC, "1" NBSP "h 00" NBSP "min 00" NBSP "s"},
{86399 * NS_SEC, "23" NBSP "h 59" NBSP "min 59" NBSP "s"},
{86400 * NS_SEC, "1" NBSP "j 00" NBSP "h 00" NBSP "min 00" NBSP "s"},
{90061 * NS_SEC, "1" NBSP "j 01" NBSP "h 01" NBSP "min 01" NBSP "s"},
{UINT64_MAX, "213503" NBSP "j 23" NBSP "h 34" NBSP "min 33" NBSP "s"}
};

static void	uptime_partitions_et_limites(void)
{
	char		out[64];
	uint32_t	i;
	int			n;

	i = 0;
	while (i < sizeof(g_rows) / sizeof(g_rows[0]))
	{
		n = fmt_uptime(g_rows[i].ns, out, sizeof(out));
		h_eq_str("texte", out, g_rows[i].want);
		h_eq_i64("longueur rendue", n, (int64_t)strlen(g_rows[i].want));
		i++;
	}
}

static void	uptime_tampon_trop_petit(void)
{
	char	out[16];

	h_eq_i64("exact", fmt_uptime(61 * NS_SEC, out, 13), 12);
	h_eq_i64("un de moins", fmt_uptime(61 * NS_SEC, out, 12), -34);
	h_eq_str("tronque", out, "1" NBSP "min 01" NBSP);
	h_eq_i64("taille 1", fmt_uptime(0, out, 1), -34);
	h_eq_str("taille 1 vide", out, "");
	out[0] = 'x';
	h_eq_i64("taille 0", fmt_uptime(0, out, 0), -34);
	h_eq_i64("taille 0 sans ecriture", out[0], 'x');
}

static void	uptime_monotone_en_longueur(void)
{
	char		out[64];
	uint64_t	s;
	size_t		prev;

	prev = 0;
	s = 0;
	while (s < 400000)
	{
		fmt_uptime(s * NS_SEC, out, sizeof(out));
		h_true(strlen(out) >= prev || s % 60 != 0, "forme stable");
		h_true(out[strlen(out) - 1] == 's', "finit par s");
		prev = strlen(out);
		s += 59;
	}
}

int	main(void)
{
	h_begin("a20/timefmt-uptime");
	h_run("uptime: partitions et limites", uptime_partitions_et_limites);
	h_run("uptime: tampon trop petit", uptime_tampon_trop_petit);
	h_run("uptime: balayage de formes", uptime_monotone_en_longueur);
	return (h_end());
}
