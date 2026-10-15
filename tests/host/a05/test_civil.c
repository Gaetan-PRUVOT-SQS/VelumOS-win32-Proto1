#include "harness.h"
#include "time_int.h"
#include "velum/err.h"

static void	civil_known(void)
{
	t_civil	c;

	h_eq_i64("1970-01-01", civil_days(1970, 1, 1), 0);
	h_eq_i64("1969-12-31", civil_days(1969, 12, 31), -1);
	h_eq_i64("2000-03-01", civil_days(2000, 3, 1), 11017);
	h_eq_i64("2038-01-19", civil_days(2038, 1, 19), 24855);
	unix_to_civil(951782400, &c);
	h_true(c.year == 2000 && c.mon == 2 && c.day == 29 && c.hour == 0,
		"29 fevrier 2000");
	unix_to_civil(4102444799, &c);
	h_true(c.year == 2099 && c.mon == 12 && c.day == 31 && c.hour == 23
		&& c.min == 59 && c.sec == 59, "31 decembre 2099 23:59:59");
	unix_to_civil(-1, &c);
	h_true(c.year == 1969 && c.mon == 12 && c.day == 31 && c.sec == 59,
		"une seconde avant l'epoque");
	h_eq_u64("jeudi 1er janvier 1970", civil_weekday(0), 4);
	h_eq_u64("mercredi 31 decembre 1969", civil_weekday(-1), 3);
	h_eq_u64("lundi 1er janvier 2024", civil_weekday(19723), 1);
}

static void	civil_valid_limits(void)
{
	t_civil	c;

	unix_to_civil(0, &c);
	h_true(civil_valid(&c), "epoque valide");
	c.year = 1969;
	h_true(!civil_valid(&c), "1969 refuse");
	c.year = 2023;
	c.mon = 2;
	c.day = 29;
	h_true(!civil_valid(&c), "29 fevrier 2023");
	c.year = 2024;
	h_true(civil_valid(&c), "29 fevrier 2024");
	c.year = 1900 + 300;
	h_true(!civil_valid(&c), "29 fevrier 2200");
	c.year = 2024;
	c.mon = 4;
	c.day = 31;
	h_true(!civil_valid(&c), "31 avril");
	c.day = 30;
	c.sec = 60;
	h_true(!civil_valid(&c), "seconde 60");
	c.sec = 0;
	c.mon = 0;
	h_true(!civil_valid(&c), "mois 0");
}

static void	civil_roundtrip(void)
{
	t_civil		c;
	int64_t		s;
	uint32_t	ok;

	ok = 1;
	s = 0;
	while (s < 4102444800)
	{
		unix_to_civil(s, &c);
		ok &= civil_valid(&c);
		ok &= (civil_to_unix(&c) == s);
		s += 86399 + (s % 7919);
	}
	h_true(ok, "unix -> civil -> unix de 1970 a 2099");
}

static void	rtc_write_limits(void)
{
	t_civil		c;
	t_rtc_raw	r;

	unix_to_civil(1704067200, &c);
	rtc_encode(&c, 0x02, true, &r);
	h_eq_u64("lundi 1er janvier 2024", r.wday, 2);
	c.year = 2100;
	h_eq_i64("2100 sans siecle", rtc_encode(&c, 0x02, false, &r), E_RANGE);
	h_eq_i64("2100 avec siecle", rtc_encode(&c, 0x02, true, &r), E_OK);
	c.year = 1999;
	h_eq_i64("1999 sans siecle", rtc_encode(&c, 0x02, false, &r), E_RANGE);
	c.day = 32;
	h_eq_i64("jour 32", rtc_encode(&c, 0x02, true, &r), E_RANGE);
}

int	main(void)
{
	h_begin("a05/civil");
	h_run("dates connues", civil_known);
	h_run("bornes de validite", civil_valid_limits);
	h_run("aller-retour", civil_roundtrip);
	h_run("bornes d'ecriture cmos", rtc_write_limits);
	return (h_end());
}
