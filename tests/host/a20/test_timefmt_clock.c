#include <stdint.h>
#include "harness.h"
#include "a20_test.h"
#include "timefmt.h"

static const t_clockrow	g_rows[] = {
{0, "00:00"},
{59999999999ull, "00:00"},
{60 * NS_SEC, "00:01"},
{3600 * NS_SEC, "01:00"},
{12 * 3600 * NS_SEC, "12:00"},
{(23 * 3600 + 59 * 60) * NS_SEC, "23:59"},
{86399 * NS_SEC, "23:59"},
{86400 * NS_SEC, "00:00"},
{(86400 + 13 * 3600 + 5 * 60) * NS_SEC, "13:05"},
{UINT64_MAX, "23:34"}
};

static void	clock_partitions_et_limites(void)
{
	char		out[CLOCK_TEXT_MAX];
	uint32_t	i;

	i = 0;
	while (i < sizeof(g_rows) / sizeof(g_rows[0]))
	{
		h_eq_i64("longueur", fmt_clock(g_rows[i].ns, out, sizeof(out)), 5);
		h_eq_str("texte", out, g_rows[i].want);
		i++;
	}
}

static void	clock_balayage_des_1440_minutes(void)
{
	char		out[CLOCK_TEXT_MAX];
	uint32_t	m;
	int			shape;

	m = 0;
	while (m < 1440)
	{
		fmt_clock((uint64_t)m * 60 * NS_SEC + 59 * NS_SEC, out, sizeof(out));
		shape = out[0] >= '0' && out[0] <= '2' && out[1] >= '0'
			&& out[1] <= '9' && out[2] == ':' && out[3] >= '0'
			&& out[3] <= '5' && out[4] >= '0' && out[4] <= '9' && !out[5];
		h_true(shape, "forme HH:MM sans AM/PM");
		h_true((out[0] - '0') * 10 + (out[1] - '0') == (int)(m / 60), "heure");
		h_true((out[3] - '0') * 10 + (out[4] - '0') == (int)(m % 60), "minute");
		m++;
	}
}

static void	clock_tampon_trop_petit(void)
{
	char	out[8];

	h_eq_i64("taille 6", fmt_clock(0, out, 6), 5);
	h_eq_i64("taille 5", fmt_clock(0, out, 5), -34);
	h_eq_str("taille 5 tronque", out, "00:0");
	h_eq_i64("taille 1", fmt_clock(0, out, 1), -34);
	h_eq_str("taille 1 vide", out, "");
	out[0] = 'x';
	h_eq_i64("taille 0", fmt_clock(0, out, 0), -34);
	h_eq_i64("taille 0 sans ecriture", out[0], 'x');
}

static void	clock_prochaine_minute(void)
{
	uint64_t	x;

	h_eq_u64("zero", ns_to_next_minute(0), 60 * NS_SEC);
	h_eq_u64("59 s", ns_to_next_minute(59 * NS_SEC), NS_SEC);
	h_eq_u64("frontiere", ns_to_next_minute(60 * NS_SEC), 60 * NS_SEC);
	h_eq_u64("frontiere moins 1", ns_to_next_minute(60 * NS_SEC - 1), 1);
	h_eq_u64("max", ns_to_next_minute(UINT64_MAX), 26290448385ull);
	x = 0;
	while (x < 7 * 60 * NS_SEC)
	{
		h_true(ns_to_next_minute(x) >= 1 && ns_to_next_minute(x)
			<= 60 * NS_SEC, "borne");
		h_true((x + ns_to_next_minute(x)) % (60 * NS_SEC) == 0, "alignee");
		x += 7777777777ull;
	}
}

int	main(void)
{
	h_begin("a20/timefmt-clock");
	h_run("clock: partitions et limites", clock_partitions_et_limites);
	h_run("clock: balayage 1440 minutes", clock_balayage_des_1440_minutes);
	h_run("clock: tampon trop petit", clock_tampon_trop_petit);
	h_run("clock: prochaine minute", clock_prochaine_minute);
	return (h_end());
}
