#include "../../../kernel/time/time_int.h"
#include "velum/err.h"
#include "velum/io.h"
#include "velum/libk.h"

#define CMOS_INDEX 0x70
#define CMOS_DATA 0x71
#define CMOS_NMI_OFF 0x7f
#define RTC_REG_A 0x0a
#define RTC_REG_B 0x0b
#define RTC_UIP_TIMEOUT_NS 10000000ull
#define RTC_READ_TRIES 5

static t_a05lock	g_rtc_lock = {0, "rtc"};

t_a05lock	*rtc_lock(void)
{
	return (&g_rtc_lock);
}

uint8_t	rtc_cmos_rd(uint8_t idx)
{
	outb(CMOS_INDEX, idx & CMOS_NMI_OFF);
	return (inb(CMOS_DATA));
}

static bool	rtc_idle(void *ctx)
{
	uint64_t	fl;
	bool		idle;

	(void)ctx;
	fl = a05_lock(&g_rtc_lock);
	idle = !(rtc_cmos_rd(RTC_REG_A) & RTC_A_UIP);
	a05_unlock(&g_rtc_lock, fl);
	return (idle);
}

static int	rtc_snap(t_rtc_raw *r, uint8_t century_reg)
{
	uint64_t	fl;

	if (wait_until(rtc_idle, NULL, RTC_UIP_TIMEOUT_NS) < 0)
		return (E_TIMEOUT);
	fl = a05_lock(&g_rtc_lock);
	r->sec = rtc_cmos_rd(0x00);
	r->min = rtc_cmos_rd(0x02);
	r->hour = rtc_cmos_rd(0x04);
	r->wday = rtc_cmos_rd(0x06);
	r->day = rtc_cmos_rd(0x07);
	r->mon = rtc_cmos_rd(0x08);
	r->year = rtc_cmos_rd(0x09);
	r->regb = rtc_cmos_rd(RTC_REG_B);
	r->cent = 0;
	if (century_reg)
		r->cent = rtc_cmos_rd(century_reg);
	a05_unlock(&g_rtc_lock, fl);
	return (E_OK);
}

int	rtc_read(t_rtc_raw *out, uint8_t century_reg)
{
	t_rtc_raw	a;
	t_rtc_raw	b;
	int			tries;

	tries = 0;
	while (tries < RTC_READ_TRIES)
	{
		if (rtc_snap(&a, century_reg) < 0 || rtc_snap(&b, century_reg) < 0)
			return (E_TIMEOUT);
		if (!memcmp(&a, &b, sizeof(a)))
		{
			*out = a;
			return (E_OK);
		}
		tries++;
	}
	return (E_IO);
}
