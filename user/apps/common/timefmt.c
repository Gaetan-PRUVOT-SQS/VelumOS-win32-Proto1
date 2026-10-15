#include "tbuf.h"
#include "timefmt.h"
#include "typo.h"

static const char	*g_unit[4] = {"j", "h", "min", "s"};

int	fmt_clock(uint64_t wall_ns, char *out, size_t size)
{
	t_tbuf		b;
	uint64_t	s;

	s = wall_ns / NS_PER_SEC;
	tb_init(&b, out, size);
	tb_num(&b, (s / 3600) % 24, 2);
	tb_putc(&b, ':');
	tb_num(&b, (s / 60) % 60, 2);
	return (tb_result(&b));
}

static void	put_unit(t_tbuf *b, uint64_t v, int started, const char *unit)
{
	int	width;

	width = 1;
	if (started)
	{
		tb_putc(b, ' ');
		width = 2;
	}
	tb_num(b, v, width);
	tb_str(b, NBSP);
	tb_str(b, unit);
}

int	fmt_uptime(uint64_t ns, char *out, size_t size)
{
	t_tbuf		b;
	uint64_t	v[4];
	int			i;
	int			started;

	v[0] = ns / NS_PER_SEC / 86400;
	v[1] = ns / NS_PER_SEC / 3600 % 24;
	v[2] = ns / NS_PER_SEC / 60 % 60;
	v[3] = ns / NS_PER_SEC % 60;
	tb_init(&b, out, size);
	i = 0;
	started = 0;
	while (i < 4)
	{
		if (started || v[i] || i == 3)
		{
			put_unit(&b, v[i], started, g_unit[i]);
			started = 1;
		}
		i++;
	}
	return (tb_result(&b));
}

uint64_t	ns_to_next_minute(uint64_t wall_ns)
{
	return (NS_PER_MINUTE - wall_ns % NS_PER_MINUTE);
}
