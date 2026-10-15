#include "gfx_int.h"

void	gfx_span_move(uint32_t *d, const uint32_t *s, size_t n)
{
	if (!gfx_span_overlap(d, s, n))
		memcpy(d, s, n * sizeof(uint32_t));
	else if ((uintptr_t)d < (uintptr_t)s)
	{
		while (n > 0)
		{
			*d = *s;
			d++;
			s++;
			n--;
		}
	}
	else
	{
		while (n > 0)
		{
			n--;
			d[n] = s[n];
		}
	}
}

void	gfx_span_blend(uint32_t *d, const uint32_t *s, size_t n)
{
	if ((uintptr_t)d > (uintptr_t)s && gfx_span_overlap(d, s, n))
	{
		while (n > 0)
		{
			n--;
			gfx_store(d + n, s[n]);
		}
		return ;
	}
	while (n > 0)
	{
		gfx_store(d, *s);
		d++;
		s++;
		n--;
	}
}
