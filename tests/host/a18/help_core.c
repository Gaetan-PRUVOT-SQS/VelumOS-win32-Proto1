#include "help.h"

int	hz_add(t_wtable *t, uint32_t style)
{
	int	slot;

	slot = wt_alloc(t, 0);
	if (slot < 0)
		return (slot);
	t->w[slot].style = style;
	t->w[slot].rect = rect_make(10, 10, 200, 100);
	wz_insert(t, slot);
	return (slot);
}

void	hz_screen(t_wtable *t)
{
	wt_init(t, rect_make(0, 0, 1024, 768));
	t->work = rect_make(0, 0, 1024, 738);
}

int	hz_cmp(const t_wtable *t, const int *owner, const uint32_t *code)
{
	t_point		p;
	int			slot;
	uint32_t	ht;
	int			bad;

	bad = 0;
	p.y = 0;
	while (p.y < t->screen.h)
	{
		p.x = 0;
		while (p.x < t->screen.w)
		{
			ht = wh_hit(t, p, &slot);
			if (slot != owner[p.y * t->screen.w + p.x]
				|| ht != code[p.y * t->screen.w + p.x])
				bad++;
			p.x++;
		}
		p.y++;
	}
	return (bad);
}

static void	paint_window(const t_wtable *t, int slot, int *owner,
	uint32_t *code)
{
	t_lunawin	lw;
	t_point		p;
	uint32_t	ht;

	wh_lunawin(t, slot, &lw);
	p.y = -1;
	while (++p.y < t->screen.h)
	{
		p.x = -1;
		while (++p.x < t->screen.w)
		{
			ht = HT_CLIENT * rect_contains(lw.outer, p);
			if (ht && wh_decorated(lw.style))
				ht = luna_hit_test(&lw, p);
			if (ht != HT_NOWHERE)
			{
				owner[p.y * t->screen.w + p.x] = slot;
				code[p.y * t->screen.w + p.x] = ht;
			}
		}
	}
}

void	hz_paint(const t_wtable *t, int *owner, uint32_t *code)
{
	uint32_t	i;

	i = 0;
	while (i < t->nz)
	{
		if (wf_visible(&t->w[t->z[i]]))
			paint_window(t, t->z[i], owner, code);
		i++;
	}
}
