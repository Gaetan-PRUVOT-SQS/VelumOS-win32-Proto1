#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fx.h"

t_span	fx_dup(t_span s, size_t len)
{
	t_span	d;
	uint8_t	*m;

	d.p = NULL;
	d.len = 0;
	m = malloc(len + (len == 0));
	if (!m)
		return (d);
	if (len)
		memcpy(m, s.p, len);
	d.p = m;
	d.len = len;
	return (d);
}

t_span	fx_load(const char *name)
{
	static uint8_t	tmp[1 << 18];
	char			path[512];
	FILE			*f;
	t_span			s;

	s.p = tmp;
	s.len = 0;
	snprintf(path, sizeof(path), "%s/%s", D02_FIXTURE, name);
	f = fopen(path, "rb");
	if (!f)
		return (fx_dup(s, 0));
	s.len = fread(tmp, 1, sizeof(tmp), f);
	fclose(f);
	return (fx_dup(s, s.len));
}

void	fx_free(t_span s)
{
	free((void *)s.p);
}

void	fx_poke(t_span s, size_t off, uint8_t v)
{
	if (off < s.len)
		((uint8_t *)s.p)[off] = v;
}

uint32_t	fx_rnd(uint32_t *seed)
{
	*seed ^= *seed << 13;
	*seed ^= *seed >> 17;
	*seed ^= *seed << 5;
	return (*seed);
}
