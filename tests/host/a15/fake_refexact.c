#include "ref.h"

static bool	exact_hit(int64_t p[2], int64_t q[2], int64_t pix[2], int64_t sgn)
{
	__int128	len;
	__int128	d;
	__int128	i;
	__int128	o;

	len = p[1] - p[0];
	d = q[1] - q[0];
	if (d < 0)
		d = -d;
	i = pix[0] - p[0];
	if (i < 0 || i > len)
		return (false);
	o = 0;
	if (len > 0)
		o = (2 * d * i + len - 1) / (2 * len);
	return ((__int128)pix[1] == q[0] + sgn * o);
}

bool	ref_on_line(t_point a, t_point b, int64_t x, int64_t y)
{
	int64_t	p[2];
	int64_t	q[2];
	int64_t	pix[2];
	bool	steep;

	steep = ref_axes(a, b, p, q);
	pix[0] = x;
	pix[1] = y;
	if (steep)
	{
		pix[0] = y;
		pix[1] = x;
	}
	return (exact_hit(p, q, pix, 1 - 2 * (q[1] < q[0])));
}
