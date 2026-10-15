#include "../../../kernel/time/time_int.h"

bool	rtc_field_dec(uint8_t v, uint8_t regb, uint32_t *out)
{
	if (regb & RTC_B_BINARY)
	{
		*out = v;
		return (true);
	}
	if ((v & 0x0f) > 9 || (v >> 4) > 9)
		return (false);
	*out = (uint32_t)(v >> 4) * 10 + (v & 0x0f);
	return (true);
}

uint8_t	rtc_field_enc(uint32_t v, uint8_t regb)
{
	if (regb & RTC_B_BINARY)
		return ((uint8_t)v);
	return ((uint8_t)(((v / 10) << 4) | (v % 10)));
}

bool	rtc_hour_dec(uint8_t h, uint8_t regb, uint32_t *out)
{
	uint32_t	v;

	if (regb & RTC_B_24H)
		return (rtc_field_dec(h, regb, out) && *out < 24);
	if (!rtc_field_dec(h & 0x7f, regb, &v) || v < 1 || v > 12)
		return (false);
	*out = v % 12;
	if (h & RTC_HOUR_PM)
		*out += 12;
	return (true);
}

uint8_t	rtc_hour_enc(uint32_t h24, uint8_t regb)
{
	uint32_t	h12;

	if (regb & RTC_B_24H)
		return (rtc_field_enc(h24, regb));
	h12 = h24 % 12;
	if (h12 == 0)
		h12 = 12;
	if (h24 >= 12)
		return (rtc_field_enc(h12, regb) | RTC_HOUR_PM);
	return (rtc_field_enc(h12, regb));
}
