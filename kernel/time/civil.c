#include "time_int.h"

static bool	civil_leap(int64_t y)
{
	return ((y % 4 == 0 && y % 100 != 0) || y % 400 == 0);
}

static uint32_t	civil_dim(int64_t y, uint32_t m)
{
	if (m == 2)
		return (28 + civil_leap(y));
	if (m == 4 || m == 6 || m == 9 || m == 11)
		return (30);
	return (31);
}

int64_t	civil_days(int64_t y, uint32_t m, uint32_t d)
{
	int64_t	era;
	int64_t	yoe;
	int64_t	doy;

	if (m <= 2)
		y--;
	era = y / 400;
	if (y < 0 && y % 400)
		era--;
	yoe = y - era * 400;
	doy = (153 * (int64_t)((m + 9) % 12) + 2) / 5 + d - 1;
	return (era * 146097 + yoe * 365 + yoe / 4 - yoe / 100 + doy - 719468);
}

void	civil_from_days(int64_t z, t_civil *out)
{
	int64_t	era;
	int64_t	doe;
	int64_t	yoe;
	int64_t	doy;
	int64_t	mp;

	z += 719468;
	era = z / 146097;
	if (z < 0 && z % 146097)
		era--;
	doe = z - era * 146097;
	yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;
	doy = doe - (365 * yoe + yoe / 4 - yoe / 100);
	mp = (5 * doy + 2) / 153;
	out->day = (uint32_t)(doy - (153 * mp + 2) / 5 + 1);
	out->mon = (uint32_t)(mp + 3);
	if (mp >= 10)
		out->mon = (uint32_t)(mp - 9);
	out->year = yoe + era * 400 + (out->mon <= 2);
}

bool	civil_valid(const t_civil *c)
{
	if (c->year < UNIX_MIN_YEAR || c->year > UNIX_MAX_YEAR)
		return (false);
	if (c->mon < 1 || c->mon > 12 || c->day < 1)
		return (false);
	if (c->day > civil_dim(c->year, c->mon))
		return (false);
	return (c->hour < 24 && c->min < 60 && c->sec < 60);
}
