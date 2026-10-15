#include <string.h>
#include "fake.h"

const t_droidview	*droid_view(const t_droid *d, uint32_t view_id)
{
	if (!d || view_id == 0 || view_id > DROID_VIEWS_MAX)
		return (NULL);
	if (d->views[view_id - 1].kind == DV_NONE)
		return (NULL);
	return (&d->views[view_id - 1]);
}

void	fd_reset(t_droid *d)
{
	memset(d, 0, sizeof(*d));
}

uint32_t	fd_add(t_droid *d, uint32_t kind, uint32_t parent)
{
	t_droidview	*v;
	t_droidview	*p;

	if (d->nviews >= DROID_VIEWS_MAX)
		return (0);
	v = &d->views[d->nviews];
	d->nviews++;
	v->kind = kind;
	v->id = d->nviews;
	v->parent = parent;
	v->orientation = DROID_VERTICAL;
	memcpy(v->text, "texte", 6);
	if (parent == 0)
		return (v->id);
	p = &d->views[parent - 1];
	if (p->nkids < DROID_KIDS_MAX)
		p->kids[p->nkids] = v->id;
	p->nkids++;
	return (v->id);
}

int	fd_inside(const t_apklay *l, t_rect zone)
{
	uint32_t	i;
	t_rect		r;

	i = 0;
	while (i < l->n)
	{
		r = l->box[i].r;
		if (r.w <= 0 || r.h <= 0 || r.x < zone.x || r.y < zone.y
			|| r.x + r.w > zone.x + zone.w || r.y + r.h > zone.y + zone.h)
			return (0);
		i++;
	}
	return (l->n <= DROID_VIEWS_MAX);
}

int	fd_disjoint(const t_apklay *l)
{
	uint32_t	i;
	uint32_t	j;
	t_rect		a;
	t_rect		b;

	i = 0;
	while (i < l->n)
	{
		j = i + 1;
		while (j < l->n)
		{
			a = l->box[i].r;
			b = l->box[j].r;
			if (a.x < b.x + b.w && b.x < a.x + a.w && a.y < b.y + b.h
				&& b.y < a.y + a.h)
				return (0);
			j++;
		}
		i++;
	}
	return (1);
}
