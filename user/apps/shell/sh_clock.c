#include "../common/platform.h"
#include "../common/tbuf.h"
#include "shell.h"

static t_civil	civil_from_days(int64_t days)
{
	t_civil	c;
	int64_t	z;
	int64_t	doe;
	int64_t	yoe;
	int64_t	doy;

	z = days + 719468;
	doe = z - z / 146097 * 146097;
	yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;
	doy = doe - (365 * yoe + yoe / 4 - yoe / 100);
	c.m = (5 * doy + 2) / 153;
	c.d = doy - (153 * c.m + 2) / 5 + 1;
	c.m = c.m + 3 - 12 * (c.m >= 10);
	c.y = yoe + z / 146097 * 400 + (c.m <= 2);
	return (c);
}

void	sh_date_fmt(uint64_t wall_ns, char *out, size_t size)
{
	t_civil	c;
	t_tbuf	b;

	c = civil_from_days((int64_t)(wall_ns / NS_PER_SEC / 86400));
	tb_init(&b, out, size);
	tb_num(&b, (uint64_t)c.d, 2);
	tb_putc(&b, '/');
	tb_num(&b, (uint64_t)c.m, 2);
	tb_putc(&b, '/');
	tb_num(&b, (uint64_t)c.y, 4);
	tb_result(&b);
}

void	sh_clock_arm(t_shell *sh)
{
	os_timer_after(sh->timer, ns_to_next_minute(os_wall_ns()), 0);
}

void	sh_clock_tick(t_shell *sh)
{
	fmt_clock(os_wall_ns(), sh->clock, sizeof(sh->clock));
	sh_date_fmt(os_wall_ns(), sh->date, sizeof(sh->date));
	sh_clock_arm(sh);
	if (sh->bar.id != 0)
		sh_bar_paint(sh);
}
