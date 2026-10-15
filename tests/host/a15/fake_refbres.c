#include <stdlib.h>
#include "ref.h"

static void	plot(t_grid *g, int64_t p, int64_t q, bool steep)
{
	if (steep)
		grid_set(g, (int32_t)q, (int32_t)p, true);
	else
		grid_set(g, (int32_t)p, (int32_t)q, true);
}

static void	walk(t_grid *g, int64_t p[2], int64_t q[2], bool steep)
{
	int64_t	dp;
	int64_t	dq;
	int64_t	d;
	int64_t	sq;

	dp = p[1] - p[0];
	dq = q[1] - q[0];
	sq = 1;
	if (dq < 0)
	{
		sq = -1;
		dq = -dq;
	}
	d = 2 * dq - dp;
	while (p[0] <= p[1])
	{
		plot(g, p[0], q[0], steep);
		if (d > 0)
		{
			q[0] += sq;
			d -= 2 * dp;
		}
		d += 2 * dq;
		p[0]++;
	}
}

bool	ref_axes(t_point a, t_point b, int64_t p[2], int64_t q[2])
{
	bool	steep;
	t_point	t;

	steep = llabs((long long)b.y - a.y) > llabs((long long)b.x - a.x);
	if ((steep && b.y < a.y) || (!steep && b.x < a.x))
	{
		t = a;
		a = b;
		b = t;
	}
	p[0] = a.x;
	p[1] = b.x;
	q[0] = a.y;
	q[1] = b.y;
	if (steep)
	{
		p[0] = a.y;
		p[1] = b.y;
		q[0] = a.x;
		q[1] = b.x;
	}
	return (steep);
}

void	ref_line(t_grid *g, t_point a, t_point b)
{
	int64_t	p[2];
	int64_t	q[2];
	bool	steep;

	steep = ref_axes(a, b, p, q);
	walk(g, p, q, steep);
}
