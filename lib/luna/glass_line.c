#include "luna_int.h"

static void	line_add(const uint32_t *in, const t_lgl *p, int32_t i, int32_t *s)
{
	uint32_t	c;
	int32_t		k;

	k = lp_clamp(i, 0, p->n - 1);
	c = in[k * p->step];
	s[0] += (int32_t)((c >> 16) & 0xff);
	s[1] += (int32_t)((c >> 8) & 0xff);
	s[2] += (int32_t)(c & 0xff);
}

static void	line_sub(const uint32_t *in, const t_lgl *p, int32_t i, int32_t *s)
{
	uint32_t	c;
	int32_t		k;

	k = lp_clamp(i, 0, p->n - 1);
	c = in[k * p->step];
	s[0] -= (int32_t)((c >> 16) & 0xff);
	s[1] -= (int32_t)((c >> 8) & 0xff);
	s[2] -= (int32_t)(c & 0xff);
}

void	lp_gb_line(const uint32_t *in, uint32_t *out, const t_lgl *p)
{
	int32_t	sum[3];
	int32_t	win;
	int32_t	i;

	sum[0] = 0;
	sum[1] = 0;
	sum[2] = 0;
	win = 2 * p->r + 1;
	i = -p->r - 1;
	while (++i <= p->r)
		line_add(in, p, i, sum);
	i = -1;
	while (++i < p->n)
	{
		out[i * p->step] = 0xff000000 | ((uint32_t)(sum[0] / win) << 16)
			| ((uint32_t)(sum[1] / win) << 8) | (uint32_t)(sum[2] / win);
		line_add(in, p, i + p->r + 1, sum);
		line_sub(in, p, i - p->r, sum);
	}
}
