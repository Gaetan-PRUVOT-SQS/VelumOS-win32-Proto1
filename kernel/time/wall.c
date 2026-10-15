#include "time_int.h"
#include "velum/acpi.h"
#include "velum/boot.h"
#include "velum/err.h"
#include "velum/klog.h"

static uint64_t	g_wall_base;

static int64_t	wall_from_rtc(void)
{
	t_rtc_raw	raw;
	t_civil		c;
	uint8_t		cent;
	int64_t		s;

	cent = acpi_info()->century_reg;
	if (rtc_read(&raw, cent) < 0 || rtc_decode(&raw, cent != 0, &c) < 0)
		return (-1);
	s = civil_to_unix(&c);
	if (s < 0 || (uint64_t)s >= WALL_MAX_NS / NS_PER_S)
		return (-1);
	return (s);
}

int	wall_init(void)
{
	int64_t		s;
	const char	*src;

	src = "RTC";
	s = wall_from_rtc();
	if (s < 0)
	{
		src = "chargeur";
		s = boot_info()->boot_unix_s;
	}
	if (s <= 0 || (uint64_t)s >= WALL_MAX_NS / NS_PER_S)
	{
		src = "valeur par défaut";
		s = (int64_t)(WALL_MIN_NS / NS_PER_S);
	}
	__atomic_store_n(&g_wall_base, (uint64_t)s * NS_PER_S - time_now_ns(),
		__ATOMIC_RELEASE);
	klog_info("time: heure murale %lld s depuis 1970 (source %s)", s, src);
	return (E_OK);
}

uint64_t	time_wall_ns(void)
{
	return (__atomic_load_n(&g_wall_base, __ATOMIC_ACQUIRE) + time_now_ns());
}

int	time_set_wall_ns(uint64_t ns)
{
	t_civil	c;
	int		rc;

	if (ns < WALL_MIN_NS || ns >= WALL_MAX_NS)
		return (E_RANGE);
	unix_to_civil((int64_t)(ns / NS_PER_S), &c);
	rc = rtc_write(&c, acpi_info()->century_reg);
	__atomic_store_n(&g_wall_base, ns - time_now_ns(), __ATOMIC_RELEASE);
	if (rc < 0)
		klog_warn("time: horloge CMOS non mise à jour (%d)", rc);
	return (rc);
}
