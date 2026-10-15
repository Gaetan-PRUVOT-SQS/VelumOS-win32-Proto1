#include "time_int.h"

#define SECS_PER_DAY 86400

int64_t	civil_to_unix(const t_civil *c)
{
	return (civil_days(c->year, c->mon, c->day) * SECS_PER_DAY
		+ (int64_t)c->hour * 3600 + (int64_t)c->min * 60 + c->sec);
}

void	unix_to_civil(int64_t s, t_civil *out)
{
	int64_t	days;
	int64_t	rem;

	days = s / SECS_PER_DAY;
	rem = s % SECS_PER_DAY;
	if (rem < 0)
	{
		rem += SECS_PER_DAY;
		days--;
	}
	civil_from_days(days, out);
	out->hour = (uint32_t)(rem / 3600);
	out->min = (uint32_t)(rem / 60 % 60);
	out->sec = (uint32_t)(rem % 60);
	out->reserved = 0;
}

uint32_t	civil_weekday(int64_t days)
{
	int64_t	w;

	w = (days + 4) % 7;
	if (w < 0)
		w += 7;
	return ((uint32_t)w);
}
