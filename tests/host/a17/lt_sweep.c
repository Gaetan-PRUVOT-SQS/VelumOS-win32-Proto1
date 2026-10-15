#include "lt_hit.h"

static void	ft_zero_counts(t_ltsweep *out)
{
	int	i;

	i = 0;
	while (i < 21)
	{
		out->count[i] = 0;
		i++;
	}
	out->edge_client = 0;
	out->edge_close = 0;
	out->edge_sys = 0;
	out->bad = 0;
	out->bad_x = 0;
	out->bad_y = 0;
}

static void	sweep_note(t_ltsweep *out, t_point p, bool ok)
{
	if (ok)
		return ;
	if (out->bad == 0)
	{
		out->bad_x = p.x;
		out->bad_y = p.y;
	}
	out->bad++;
}

static bool	sweep_check(const t_lunawin *w, t_rect cl, t_point p, uint32_t h)
{
	t_point	m;
	bool	framed;

	framed = (w->style & WS_CAPTION) && !(w->style & 0x0e00);
	if (!lt_ht_valid(h))
		return (false);
	if (lt_ht_edge(h))
	{
		m = lp_pt(w->outer.x + w->outer.w - 1 - (p.x - w->outer.x), p.y);
		if (lt_ht_mirror(h) != luna_hit_test(w, m))
			return (false);
		if (!(w->style & WS_SIZEBOX)
			|| (w->maximized && !(w->style & WS_TOOLWINDOW)))
			return (false);
	}
	if (h == HT_CLIENT && !lp_in(cl, p))
		return (false);
	if (lp_in(cl, p) && h != HT_CLIENT && !lt_ht_edge(h))
		return (false);
	return (framed || h == HT_CLIENT);
}

static void	sweep_edge(const t_lunawin *w, t_point p, t_ltsweep *out)
{
	out->edge_client += lp_in(luna_window_client(w), p);
	out->edge_close += lp_in(luna_button_rect(w, HT_CLOSE), p);
	out->edge_sys += lp_in(luna_button_rect(w, HT_SYSMENU), p);
}

void	lt_sweep(const t_lunawin *w, int32_t step, t_ltsweep *out)
{
	t_rect		cl;
	t_point		p;
	uint32_t	h;

	ft_zero_counts(out);
	cl = luna_window_client(w);
	p.y = w->outer.y;
	while (p.y < w->outer.y + w->outer.h)
	{
		p.x = w->outer.x;
		while (p.x < w->outer.x + w->outer.w)
		{
			if (lt_sweep_visit(w, p, step))
			{
				h = luna_hit_test(w, p);
				if (h < 21)
					out->count[h]++;
				if (lt_ht_edge(h))
					sweep_edge(w, p, out);
				sweep_note(out, p, h < 21 && sweep_check(w, cl, p, h));
			}
			p.x++;
		}
		p.y++;
	}
}
