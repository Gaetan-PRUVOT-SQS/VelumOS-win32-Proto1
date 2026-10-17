#include "luna_ctrl.h"

static void	edit_colors(bool enabled, t_color *fill, t_color *ring)
{
	t_cpal	pal;

	pal = lp_pal();
	*fill = pal->input;
	*ring = lp_tone_alpha(pal->muted, 0x73);
	if (enabled)
		return ;
	*fill = lp_tone_alpha(*fill, (*fill >> 24) / 2);
	*ring = lp_tone_alpha(pal->dim, 0x73);
}

void	luna_edit_frame(t_surface *s, t_rect r, bool enabled)
{
	t_lrfill	f;
	t_color		fill;
	t_color		ring;
	int32_t		rad;

	if (!lp_ok(s))
		return ;
	r = lp_clean(r);
	edit_colors(enabled, &fill, &ring);
	rad = lp_min(LCT_RAD_FIELD, r.h / 2);
	f = lp_rmake(lp_inset(r, 1), rad - 1, fill, fill);
	lp_rfill(s, &f);
	f = lp_rmake(r, rad, ring, ring);
	lp_rring(s, &f, 1);
}

static void	group_part(t_surface *s, const t_lrfill *f, t_rect part)
{
	t_rect	keep;

	keep = s->clip;
	s->clip = lp_isect(keep, part);
	lp_rring(s, f, 1);
	s->clip = keep;
}

static void	group_draw(t_surface *s, const t_lrfill *f, int32_t label_w)
{
	int32_t	gap0;
	int32_t	gap1;

	if (label_w == 0)
	{
		lp_rring(s, f, 1);
		return ;
	}
	gap0 = f->r.x + lm_detail()->group_pad;
	gap1 = gap0 + label_w;
	group_part(s, f, lp_rect(f->r.x, f->r.y, gap0 - f->r.x, f->r.h));
	group_part(s, f, lp_rect(gap1, f->r.y, f->r.x + f->r.w - gap1, f->r.h));
	group_part(s, f, lp_rect(gap0, f->r.y + 1, label_w, f->r.h - 1));
}

void	luna_groupbox(t_surface *s, t_rect r, int32_t label_w)
{
	t_lrfill	f;
	t_color		c;
	int32_t		dy;

	r = lp_clean(r);
	label_w = lp_clamp(label_w, 0, LP_COORD_MAX);
	dy = lm_detail()->group_line_y;
	if (!lp_ok(s) || r.w < 8 || r.h < 8 || r.h - dy < 8)
		return ;
	c = lp_tone_alpha(lp_pal()->muted, 0x4d);
	f = lp_rmake(lp_rect(r.x, r.y + dy, r.w, r.h - dy), LCT_RAD_GROUP, c, c);
	group_draw(s, &f, label_w);
}
