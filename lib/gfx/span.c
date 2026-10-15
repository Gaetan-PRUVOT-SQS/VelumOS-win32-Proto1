#include "gfx_int.h"

void	gfx_span_fill(uint32_t *d, size_t n, t_color c)
{
	while (n > 0)
	{
		*d = c;
		d++;
		n--;
	}
}

void	gfx_span_copy(uint32_t *d, const uint32_t *s, size_t n)
{
	memcpy(d, s, n * sizeof(uint32_t));
}

void	gfx_span_color(uint32_t *d, size_t n, t_color c)
{
	if ((c >> 24) == 255)
		gfx_span_fill(d, n, c);
	else if ((c >> 24) != 0)
	{
		while (n > 0)
		{
			*d = gfx_blend_px(*d, c);
			d++;
			n--;
		}
	}
}

bool	gfx_span_overlap(const uint32_t *d, const uint32_t *s, size_t n)
{
	uintptr_t	a;
	uintptr_t	b;
	uintptr_t	len;

	a = (uintptr_t)d;
	b = (uintptr_t)s;
	len = n * sizeof(uint32_t);
	return (a < b + len && b < a + len);
}
