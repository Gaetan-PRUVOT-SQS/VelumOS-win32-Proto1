#include "../../../kernel/time/time_int.h"
#include "velum/err.h"

#define CENTURY_MIN 19
#define CENTURY_MAX 99

int	rtc_decode(const t_rtc_raw *r, bool has_century, t_civil *out)
{
	uint32_t	yy;
	uint32_t	cc;

	if (!rtc_field_dec(r->sec, r->regb, &out->sec)
		|| !rtc_field_dec(r->min, r->regb, &out->min)
		|| !rtc_hour_dec(r->hour, r->regb, &out->hour)
		|| !rtc_field_dec(r->day, r->regb, &out->day)
		|| !rtc_field_dec(r->mon, r->regb, &out->mon)
		|| !rtc_field_dec(r->year, r->regb, &yy) || yy > 99)
		return (E_IO);
	cc = RTC_MIN_YEAR / 100;
	if (has_century && (!rtc_field_dec(r->cent, r->regb, &cc)
			|| cc < CENTURY_MIN || cc > CENTURY_MAX))
		return (E_IO);
	out->year = (int64_t)cc * 100 + yy;
	out->reserved = 0;
	if (!civil_valid(out))
		return (E_IO);
	return (E_OK);
}

int	rtc_encode(const t_civil *c, uint8_t regb, bool has_century,
	t_rtc_raw *out)
{
	uint32_t	wday;

	if (!civil_valid(c) || (!has_century && (c->year < RTC_MIN_YEAR
				|| c->year > RTC_MAX_YEAR)))
		return (E_RANGE);
	wday = civil_weekday(civil_days(c->year, c->mon, c->day));
	out->regb = regb;
	out->sec = rtc_field_enc(c->sec, regb);
	out->min = rtc_field_enc(c->min, regb);
	out->hour = rtc_hour_enc(c->hour, regb);
	out->wday = rtc_field_enc(wday + 1, regb);
	out->day = rtc_field_enc(c->day, regb);
	out->mon = rtc_field_enc(c->mon, regb);
	out->year = rtc_field_enc((uint32_t)(c->year % 100), regb);
	out->cent = rtc_field_enc((uint32_t)(c->year / 100), regb);
	return (E_OK);
}
