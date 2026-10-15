#include "ref.h"

bool	in_rect(t_rect r, int64_t x, int64_t y)
{
	if (r.w <= 0 || r.h <= 0)
		return (false);
	return (x >= r.x && x < (int64_t)r.x + r.w && y >= r.y
		&& y < (int64_t)r.y + r.h);
}

bool	in_view(const t_surface *s, int64_t x, int64_t y)
{
	if (s->px == NULL || s->w <= 0 || s->h <= 0 || s->stride < s->w)
		return (false);
	if (x < 0 || y < 0 || x >= s->w || y >= s->h)
		return (false);
	return (in_rect(s->clip, x, y));
}

size_t	img_diff(const uint32_t *a, const uint32_t *b, size_t n)
{
	size_t	i;

	i = 0;
	while (i < n && a[i] == b[i])
		i++;
	return (i);
}

void	img_pattern(uint32_t *p, size_t n, uint32_t seed)
{
	size_t	i;

	i = 0;
	while (i < n)
	{
		p[i] = (uint32_t)((i + seed) * 2654435761u) ^ 0xff0a0b0cu;
		i++;
	}
}

t_rect	clip_variant(int k, int32_t w, int32_t h)
{
	if (k == 0)
		return (rect_make(0, 0, w, h));
	if (k == 1)
		return (rect_make(1, 1, w - 2, h - 2));
	if (k == 2)
		return (rect_make(-2, -2, w / 2 + 2, h / 2 + 2));
	if (k == 3)
		return (rect_make(w + 5, h + 5, 3, 3));
	if (k == 4)
		return (rect_make(0, 0, 0, 0));
	if (k == 5)
		return (rect_make(INT32_MIN, INT32_MIN, INT32_MAX, INT32_MAX));
	if (k == 6)
		return (rect_make(w / 2, 0, w, -4));
	return (rect_make(-1, h / 2, w + 2, h));
}
