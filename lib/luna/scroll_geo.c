#include "luna_int.h"

static t_rect	sg_rect(const t_rect *r, bool vert, int32_t off, int32_t len)
{
	if (vert)
		return (lp_rect(r->x, r->y + off, r->w, len));
	return (lp_rect(r->x + off, r->y, len, r->h));
}

static t_rect	sg_thumb(const t_lunascroll *sb, bool vert, int32_t a,
					int32_t trk)
{
	int64_t	len;
	int64_t	off;
	int64_t	range;

	if (sb->total <= 0 || sb->page <= 0 || sb->page >= sb->total || trk <= 0)
		return (lp_rect(sb->r.x, sb->r.y, 0, 0));
	len = (int64_t)trk * sb->page / sb->total;
	len = lp_clamp((int32_t)len, lm_detail()->thumb_min, trk);
	range = sb->total - sb->page;
	off = (int64_t)(trk - len) * lp_clamp(sb->pos, 0, (int32_t)range) / range;
	return (sg_rect(&sb->r, vert, a + (int32_t)off, (int32_t)len));
}

void	luna_scroll_layout(const t_lunascroll *src, t_scrollparts *out)
{
	t_lunascroll	sb;
	bool			vert;
	int32_t			len;
	int32_t			a;

	if (src == NULL || out == NULL)
		return ;
	sb = *src;
	sb.r = lp_clean(src->r);
	vert = sb.r.h >= sb.r.w;
	len = sb.r.w;
	if (vert)
		len = sb.r.h;
	a = lp_min(lm_metrics()->scroll_w, len / 2);
	out->up = sg_rect(&sb.r, vert, 0, a);
	out->down = sg_rect(&sb.r, vert, len - a, a);
	out->track = sg_rect(&sb.r, vert, a, len - 2 * a);
	out->thumb = sg_thumb(&sb, vert, a, len - 2 * a);
}
