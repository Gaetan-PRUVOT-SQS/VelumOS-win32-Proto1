#include "d07.h"

uint32_t	d07_rand(uint32_t *s)
{
	*s ^= *s << 13;
	*s ^= *s >> 17;
	*s ^= *s << 5;
	return (*s);
}

void	d07_block_bounds(t_span f, size_t *lo, size_t *hi)
{
	t_apk	a;

	*lo = 0;
	*hi = 0;
	if (apk_open(&a, f) < 0)
		return ;
	*hi = a.zip.cd_off;
	*lo = *hi - 8 - (size_t)f.p[*hi - 24] - ((size_t)f.p[*hi - 23] << 8);
}

int	d07_try(t_span f)
{
	t_apk			a;
	t_apksig		s;
	t_apkmanifest	m;
	int				r;

	r = apk_open(&a, f);
	if (r < 0)
		return (r);
	apk_manifest(&a, &m);
	return (apk_verify(&a, &s));
}
