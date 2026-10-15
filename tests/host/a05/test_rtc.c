#include "harness.h"
#include "time_int.h"
#include "velum/err.h"

static const uint8_t	g_cases[][9] = {
{0x59, 0x59, 0x23, 0x31, 0x12, 0x99, 0x20, 0x02, 23},
{59, 59, 23, 31, 12, 99, 20, 0x06, 23},
{0, 0, 0x12, 1, 1, 0x24, 0x20, 0x00, 0},
{0, 0, 0x92, 1, 1, 0x24, 0x20, 0x00, 12},
{0, 0, 0x91, 1, 1, 0x24, 0x20, 0x00, 23},
{0, 0, 0x01, 1, 1, 0x24, 0x20, 0x00, 1},
{0, 0, 0x8c, 1, 1, 24, 20, 0x04, 12},
{0, 0, 0x12, 0x29, 0x02, 0x24, 0x20, 0x02, 12},
{0, 0, 0x00, 0x29, 0x02, 0x00, 0x20, 0x02, 0},
{0, 0, 0x00, 0x01, 0x01, 0x99, 0x19, 0x02, 0}
};
static const uint8_t	g_bad[][8] = {
{0, 0, 0x00, 1, 1, 0x24, 0x20, 0x00},
{0, 0, 0x13, 1, 1, 0x24, 0x20, 0x00},
{0x5a, 0, 0, 1, 1, 0x24, 0x20, 0x02},
{0, 0, 0, 0x29, 0x02, 0x23, 0x20, 0x02},
{0, 0, 0, 0x29, 0x02, 0x00, 0x21, 0x02},
{0, 0, 0, 1, 1, 0x69, 0x19, 0x02},
{0, 0, 0, 1, 1, 0x24, 0x1a, 0x02},
{0, 0, 0, 0, 1, 0x24, 0x20, 0x02},
{0, 0, 0, 1, 0x13, 0x24, 0x20, 0x02},
{0, 0, 0, 1, 1, 160, 20, 0x06},
{0, 0, 0x24, 1, 1, 0x24, 0x20, 0x02}
};

static void	fill(t_rtc_raw *r, const uint8_t *v)
{
	r->sec = v[0];
	r->min = v[1];
	r->hour = v[2];
	r->day = v[3];
	r->mon = v[4];
	r->year = v[5];
	r->cent = v[6];
	r->regb = v[7];
	r->wday = 1;
}

static void	rtc_valid(void)
{
	t_rtc_raw	r;
	t_civil		c;
	uint32_t	k;

	k = 0;
	while (k < sizeof(g_cases) / sizeof(g_cases[0]))
	{
		fill(&r, g_cases[k]);
		h_eq_i64("decodage valide", rtc_decode(&r, true, &c), E_OK);
		h_eq_u64("heure 24 h", c.hour, g_cases[k][8]);
		k++;
	}
	fill(&r, g_cases[0]);
	rtc_decode(&r, true, &c);
	h_true(c.year == 2099 && c.mon == 12 && c.day == 31 && c.sec == 59,
		"31 decembre 2099 23:59:59");
	fill(&r, g_cases[8]);
	rtc_decode(&r, true, &c);
	h_true(c.year == 2000 && c.day == 29, "29 fevrier 2000");
	fill(&r, g_cases[9]);
	rtc_decode(&r, true, &c);
	h_eq_i64("siecle 19", c.year, 1999);
	r.cent = 0x55;
	h_eq_i64("sans siecle", rtc_decode(&r, false, &c), E_OK);
	h_eq_i64("sans siecle 20aa", c.year, 2099);
}

static void	rtc_invalid(void)
{
	t_rtc_raw	r;
	t_civil		c;
	uint32_t	k;

	k = 0;
	while (k < sizeof(g_bad) / sizeof(g_bad[0]))
	{
		fill(&r, g_bad[k]);
		h_eq_i64("decodage refuse", rtc_decode(&r, true, &c), E_IO);
		k++;
	}
}

static void	rtc_roundtrip(void)
{
	t_civil		c;
	t_civil		d;
	t_rtc_raw	r;
	uint32_t	ok;

	ok = 1;
	unix_to_civil(946684800, &c);
	while (c.year < 2100)
	{
		r.regb = (uint8_t)((c.day + c.hour) & 0x6);
		ok &= (rtc_encode(&c, r.regb, (c.mon & 1) != 0, &r) == E_OK);
		ok &= (rtc_decode(&r, (c.mon & 1) != 0, &d) == E_OK);
		ok &= (civil_to_unix(&c) == civil_to_unix(&d));
		unix_to_civil(civil_to_unix(&c) + 3599 * 7 + 13, &c);
	}
	h_true(ok, "encodage puis decodage, 4 formats, 2000-2099");
}

int	main(void)
{
	h_begin("a05/rtc");
	h_run("decodage cmos valide", rtc_valid);
	h_run("decodage cmos refuse", rtc_invalid);
	h_run("aller-retour et bornes d'ecriture", rtc_roundtrip);
	return (h_end());
}
