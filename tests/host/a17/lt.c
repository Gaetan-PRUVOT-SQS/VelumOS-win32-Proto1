#include <stdlib.h>
#include "lt.h"

int	lt_open(t_lt *t, int32_t w, int32_t h)
{
	size_t	stride;
	size_t	i;

	stride = (size_t)w + 2 * LT_GUARD;
	t->count = stride * ((size_t)h + 2 * LT_GUARD);
	t->buf = malloc(t->count * sizeof(uint32_t));
	if (t->buf == NULL)
		return (-1);
	i = 0;
	while (i < t->count)
	{
		t->buf[i] = LT_SENTINEL;
		i++;
	}
	t->w = w;
	t->h = h;
	t->s.px = t->buf + LT_GUARD * stride + LT_GUARD;
	t->s.w = w;
	t->s.h = h;
	t->s.stride = (int32_t)stride;
	t->s.clip.x = 0;
	t->s.clip.y = 0;
	t->s.clip.w = w;
	t->s.clip.h = h;
	return (0);
}

void	lt_close(t_lt *t)
{
	free(t->buf);
	t->buf = NULL;
}

void	lt_clear(t_lt *t, t_color c)
{
	int32_t	x;
	int32_t	y;

	y = 0;
	while (y < t->h)
	{
		x = 0;
		while (x < t->w)
		{
			t->s.px[(size_t)y * (size_t)t->s.stride + (size_t)x] = c;
			x++;
		}
		y++;
	}
}

int	lt_guard_ok(const t_lt *t)
{
	size_t	stride;
	size_t	i;
	size_t	x;
	size_t	y;

	stride = (size_t)t->w + 2 * LT_GUARD;
	i = 0;
	while (i < t->count)
	{
		x = i % stride;
		y = i / stride;
		if ((x < LT_GUARD || x >= LT_GUARD + (size_t)t->w
				|| y < LT_GUARD || y >= LT_GUARD + (size_t)t->h)
			&& t->buf[i] != LT_SENTINEL)
			return (0);
		i++;
	}
	return (1);
}

uint64_t	lt_hash(const t_surface *s)
{
	uint64_t	h;
	int32_t		x;
	int32_t		y;

	h = 1469598103934665603ULL;
	y = 0;
	while (y < s->h)
	{
		x = 0;
		while (x < s->w)
		{
			h ^= s->px[(size_t)y * (size_t)s->stride + (size_t)x];
			h *= 1099511628211ULL;
			x++;
		}
		y++;
	}
	return (h);
}
