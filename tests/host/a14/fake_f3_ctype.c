#include "fake_f3.h"
#include "string.h"

void	f3_ctype_build(const t_f3_ctype *d, uint8_t *exp)
{
	int	c;

	c = 0;
	while (c < 256)
	{
		exp[c] = 0;
		if (d->ref && c && strchr(d->ref, c))
			exp[c] = 1;
		if (c >= d->lo && c <= d->hi)
			exp[c] = 1;
		if (c == d->extra)
			exp[c] = 1;
		c++;
	}
}
