#include "apklay.h"

static int32_t	place(t_apklay *l, const t_droid *d, uint32_t id, t_rect z);

static int32_t	column(t_apklay *l, const t_droid *d, const t_droidview *v,
		t_rect z)
{
	uint32_t	i;
	uint32_t	n;
	int32_t		y;
	int32_t		used;

	n = v->nkids;
	if (n > DROID_KIDS_MAX)
		n = DROID_KIDS_MAX;
	i = 0;
	y = 0;
	while (i < n && y < z.h)
	{
		used = place(l, d, v->kids[i],
				(t_rect){z.x, z.y + y, z.w, z.h - y});
		if (used > 0)
			y += used + APKLAY_MARGIN;
		i++;
	}
	if (y > 0)
		y -= APKLAY_MARGIN;
	return (y);
}

static int32_t	row(t_apklay *l, const t_droid *d, const t_droidview *v,
		t_rect z)
{
	uint32_t	i;
	int32_t		n;
	int32_t		w;
	int32_t		used;
	int32_t		h;

	n = DROID_KIDS_MAX;
	if (v->nkids < DROID_KIDS_MAX)
		n = (int32_t)v->nkids;
	h = 0;
	if (n == 0)
		return (0);
	w = (z.w - (n - 1) * APKLAY_MARGIN) / n;
	i = 0;
	while (w >= APKLAY_MIN_W && i < (uint32_t)n)
	{
		used = place(l, d, v->kids[i], (t_rect){z.x + (int32_t)i
				* (w + APKLAY_MARGIN), z.y, w, z.h});
		if (used > h)
			h = used;
		i++;
	}
	return (h);
}

static int32_t	place(t_apklay *l, const t_droid *d, uint32_t id, t_rect z)
{
	const t_droidview	*v;
	uintptr_t			off;
	int32_t				h;

	v = droid_view(d, id);
	if (!v || z.w < APKLAY_MIN_W || z.h <= 0)
		return (0);
	off = (uintptr_t)v - (uintptr_t)d->views;
	if ((uintptr_t)v < (uintptr_t)d->views || off % sizeof(*v) != 0
		|| off / sizeof(*v) >= DROID_VIEWS_MAX
		|| (l->seen >> (off / sizeof(*v)) & 1))
		return (0);
	l->seen |= 1ull << (off / sizeof(*v));
	if (v->kind == DV_LINEAR && v->orientation == DROID_HORIZONTAL)
		return (row(l, d, v, z));
	if (v->kind == DV_LINEAR)
		return (column(l, d, v, z));
	h = APKLAY_TEXT_H;
	if (v->kind == DV_BUTTON)
		h = APKLAY_BUTTON_H;
	if ((v->kind != DV_TEXT && v->kind != DV_BUTTON) || z.h < h)
		return (0);
	l->box[l->n] = (t_apkbox){id, v->kind, {z.x, z.y, z.w, h}};
	l->n++;
	return (h);
}

void	apklay_run(const t_droid *d, t_rect zone, t_apklay *out)
{
	out->n = 0;
	out->seen = 0;
	if (!d || zone.w <= 2 * APKLAY_MARGIN || zone.h <= 2 * APKLAY_MARGIN
		|| zone.w > APKLAY_ZONE_MAX || zone.h > APKLAY_ZONE_MAX
		|| zone.x < -APKLAY_ZONE_MAX || zone.x > APKLAY_ZONE_MAX
		|| zone.y < -APKLAY_ZONE_MAX || zone.y > APKLAY_ZONE_MAX)
		return ;
	zone.x += APKLAY_MARGIN;
	zone.y += APKLAY_MARGIN;
	zone.w -= 2 * APKLAY_MARGIN;
	zone.h -= 2 * APKLAY_MARGIN;
	place(out, d, d->root, zone);
}
