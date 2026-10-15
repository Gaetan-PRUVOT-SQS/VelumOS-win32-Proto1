#include "fat.h"

static uint64_t	days_from_civil(uint32_t y, uint32_t m, uint32_t d)
{
	uint32_t	era;
	uint32_t	yoe;
	uint32_t	doy;
	uint32_t	mp;

	if (m <= 2)
		y--;
	era = y / 400;
	yoe = y - era * 400;
	mp = m - 3;
	if (m <= 2)
		mp = m + 9;
	doy = (153 * mp + 2) / 5 + d - 1;
	return ((uint64_t)era * 146097 + yoe * 365 + yoe / 4 - yoe / 100 + doy
		- 719468);
}

static void	civil_from_days(uint64_t z, uint32_t *ymd)
{
	uint64_t	era;
	uint32_t	doe;
	uint32_t	yoe;
	uint32_t	doy;
	uint32_t	mp;

	z += 719468;
	era = z / 146097;
	doe = (uint32_t)(z - era * 146097);
	yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;
	doy = doe - (365 * yoe + yoe / 4 - yoe / 100);
	mp = (5 * doy + 2) / 153;
	ymd[2] = doy - (153 * mp + 2) / 5 + 1;
	ymd[1] = mp + 3;
	if (mp >= 10)
		ymd[1] = mp - 9;
	ymd[0] = (uint32_t)(yoe + era * 400);
	if (ymd[1] <= 2)
		ymd[0]++;
}

void	fat_time_pack(uint64_t ns, uint16_t *date, uint16_t *time)
{
	uint64_t	s;
	uint32_t	ymd[3];
	uint32_t	rem;

	s = ns / 1000000000ull;
	civil_from_days(s / 86400, ymd);
	rem = (uint32_t)(s % 86400);
	if (ymd[0] < 1980)
	{
		*date = (1 << 5) | 1;
		*time = 0;
		return ;
	}
	if (ymd[0] > 2107)
	{
		*date = (127 << 9) | (12 << 5) | 31;
		*time = (23 << 11) | (59 << 5) | 29;
		return ;
	}
	*date = (uint16_t)(((ymd[0] - 1980) << 9) | (ymd[1] << 5) | ymd[2]);
	*time = (uint16_t)(((rem / 3600) << 11) | ((rem / 60 % 60) << 5)
			| (rem % 60 / 2));
}

uint64_t	fat_time_unpack(uint32_t date, uint32_t time)
{
	uint32_t	m;
	uint32_t	d;
	uint64_t	s;

	m = (date >> 5) & 15;
	d = date & 31;
	if (m < 1 || m > 12 || d < 1)
		return (0);
	s = days_from_civil(1980 + (date >> 9), m, d) * 86400;
	if ((time >> 11) <= 23 && ((time >> 5) & 63) <= 59 && (time & 31) <= 29)
		s += (time >> 11) * 3600 + ((time >> 5) & 63) * 60 + (time & 31) * 2;
	return (s * 1000000000ull);
}
