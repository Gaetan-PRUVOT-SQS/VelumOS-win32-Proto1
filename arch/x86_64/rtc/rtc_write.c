#include "../../../kernel/time/time_int.h"
#include "velum/err.h"
#include "velum/io.h"

#define CMOS_INDEX 0x70
#define CMOS_DATA 0x71
#define CMOS_NMI_OFF 0x7f
#define RTC_REG_B 0x0b

static void	cmos_put(uint8_t idx, uint8_t v)
{
	outb(CMOS_INDEX, idx & CMOS_NMI_OFF);
	outb(CMOS_DATA, v);
}

static void	rtc_store(const t_rtc_raw *r, uint8_t century_reg)
{
	cmos_put(RTC_REG_B, r->regb | RTC_B_SET);
	cmos_put(0x00, r->sec);
	cmos_put(0x02, r->min);
	cmos_put(0x04, r->hour);
	cmos_put(0x06, r->wday);
	cmos_put(0x07, r->day);
	cmos_put(0x08, r->mon);
	cmos_put(0x09, r->year);
	if (century_reg)
		cmos_put(century_reg, r->cent);
	cmos_put(RTC_REG_B, r->regb & ~RTC_B_SET);
}

int	rtc_write(const t_civil *c, uint8_t century_reg)
{
	t_rtc_raw	raw;
	uint64_t	fl;
	int			rc;

	fl = a05_lock(rtc_lock());
	rc = rtc_encode(c, rtc_cmos_rd(RTC_REG_B) & ~RTC_B_SET, century_reg != 0,
			&raw);
	if (rc == E_OK)
		rtc_store(&raw, century_reg);
	a05_unlock(rtc_lock(), fl);
	return (rc);
}
