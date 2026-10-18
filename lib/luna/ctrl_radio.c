#include "luna_ctrl.h"
#include "velum/theme.h"

#define RADIO_SUB 4
#define RADIO_HUITIEME 8
#define RADIO_FILET_FIN 12
#define RADIO_FILET_EPAIS 32

static uint32_t	radio_couverture(int32_t box, t_point q, int32_t filet)
{
	uint32_t	n;
	int32_t		i;
	int32_t		dx;
	int32_t		dy;
	int64_t		d2;

	n = 0;
	i = 0;
	while (i < RADIO_SUB * RADIO_SUB)
	{
		dx = (q.x * RADIO_SUB + i % RADIO_SUB) * 2 + 1 - box * RADIO_SUB;
		dy = (q.y * RADIO_SUB + i / RADIO_SUB) * 2 + 1 - box * RADIO_SUB;
		d2 = (int64_t)dx * dx + (int64_t)dy * dy;
		if (d2 <= (int64_t)box * RADIO_SUB * box * RADIO_SUB
			&& d2 > (int64_t)(box * RADIO_SUB - filet)
			* (box * RADIO_SUB - filet))
			n++;
		i++;
	}
	return (n);
}

static t_color	radio_couleur_filet(const t_lchoice *c)
{
	t_cpal	pal;

	pal = lp_pal();
	if (c->st == LS_DISABLED)
		return (lp_tone_alpha(pal->dim, 0xa6));
	if (c->on || c->st == LS_HOT)
		return (pal->accent);
	if (lp_dark())
		return (VT_RADIO_BORDER_D);
	return (VT_RADIO_BORDER);
}

static void	radio_filet(t_surface *s, const t_lchoice *c)
{
	t_rect	vis;
	t_point	q;
	t_color	col;
	int32_t	filet;

	col = radio_couleur_filet(c);
	filet = c->on ? RADIO_FILET_EPAIS : RADIO_FILET_FIN;
	vis = lp_visible(s, c->r);
	q.y = vis.y;
	while (q.y < vis.y + vis.h)
	{
		q.x = vis.x;
		while (q.x < vis.x + vis.w)
		{
			lp_plot(s, q, col, radio_couverture(c->r.w,
					lp_pt(q.x - c->r.x, q.y - c->r.y), filet));
			q.x++;
		}
		q.y++;
	}
}

void	lp_radio(t_surface *s, const t_lchoice *c)
{
	t_lrfill	f;
	t_color		ring;

	if (!lp_ok(s) || c == NULL || c->r.w <= 0 || c->r.h != c->r.w)
		return ;
	lp_choice_colors(c, &f, &ring);
	f.r = c->r;
	f.rad = c->r.w / 2;
	lp_rfill(s, &f);
	radio_filet(s, c);
}
