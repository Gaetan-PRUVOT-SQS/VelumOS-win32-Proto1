#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "d01.h"

t_blob	d01_load(const char *name)
{
	t_blob	b;
	char	path[512];
	FILE	*f;
	long	n;

	snprintf(path, sizeof(path), "%s/%s", D01_FIX, name);
	f = fopen(path, "rb");
	if (f == NULL || fseek(f, 0, SEEK_END) != 0)
		exit(2);
	n = ftell(f);
	rewind(f);
	if (n <= 0)
		exit(2);
	b.len = (size_t)n;
	b.p = malloc(b.len);
	if (b.p == NULL || fread(b.p, 1, b.len, f) != b.len)
		exit(2);
	fclose(f);
	return (b);
}

uint32_t	d01_u32(const uint8_t *p)
{
	return ((uint32_t)p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16)
		| ((uint32_t)p[3] << 24));
}

uint8_t	*d01_dup(const uint8_t *p, size_t n)
{
	uint8_t	*q;

	q = malloc(n);
	if (q == NULL && n > 0)
		exit(2);
	if (n > 0)
		memcpy(q, p, n);
	return (q);
}

int	d01_next(const t_blob *pack, size_t *off, t_rec *r)
{
	if (*off + 12 > pack->len)
		return (0);
	r->code = (int32_t)d01_u32(pack->p + *off);
	r->alen = d01_u32(pack->p + *off + 4);
	r->blen = d01_u32(pack->p + *off + 8);
	r->a = pack->p + *off + 12;
	r->b = r->a + r->alen;
	*off += 12 + r->alen + r->blen;
	return (1);
}

uint32_t	d01_rand(uint32_t *state)
{
	*state ^= *state << 13;
	*state ^= *state >> 17;
	*state ^= *state << 5;
	return (*state);
}
