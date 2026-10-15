#include "apk_int.h"

uint32_t	apk_rd32(const uint8_t *p)
{
	return ((uint32_t)p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16)
		| ((uint32_t)p[3] << 24));
}

uint64_t	apk_rd64(const uint8_t *p)
{
	return ((uint64_t)apk_rd32(p) | ((uint64_t)apk_rd32(p + 4) << 32));
}

int	apk_lp(t_span *in, t_span *out)
{
	uint32_t	n;

	*out = (t_span){NULL, 0};
	if (in->len < 4)
		return (E_ACCES);
	n = apk_rd32(in->p);
	if (n > in->len - 4)
		return (E_ACCES);
	*out = (t_span){in->p + 4, n};
	*in = (t_span){in->p + 4 + n, in->len - 4 - n};
	return (0);
}

int	apk_text_clean(const char *s)
{
	size_t	i;

	i = 0;
	while (s[i])
	{
		if ((unsigned char)s[i] < 0x20 || s[i] == 0x7f)
			return (E_INVAL);
		i++;
	}
	return (0);
}
