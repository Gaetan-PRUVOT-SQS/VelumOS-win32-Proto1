#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fake.h"
#include "dex_int.h"

static uint8_t	g_buf[65536];
static size_t	g_len;

t_span	fake_fixture(void)
{
	FILE	*f;
	t_span	s;

	if (g_len == 0)
	{
		f = fopen(D03_FIXTURE, "rb");
		if (f)
		{
			g_len = fread(g_buf, 1, sizeof(g_buf), f);
			fclose(f);
		}
	}
	s.p = g_buf;
	s.len = g_len;
	return (s);
}

uint8_t	*fake_copy(t_span s, size_t n)
{
	uint8_t	*p;

	p = malloc(n + (n == 0));
	if (!p)
		abort();
	if (n > s.len)
		n = s.len;
	memcpy(p, s.p, n);
	return (p);
}

void	fake_seal(uint8_t *p, size_t n)
{
	uint32_t	sum;

	if (n < 12)
		return ;
	sum = dex_adler32(p + 12, n - 12);
	p[8] = (uint8_t)sum;
	p[9] = (uint8_t)(sum >> 8);
	p[10] = (uint8_t)(sum >> 16);
	p[11] = (uint8_t)(sum >> 24);
}

int	fake_patched(size_t off, uint32_t v, uint32_t width)
{
	t_span		s;
	uint8_t		*p;
	t_dex		d;
	uint32_t	i;
	int			r;

	s = fake_fixture();
	p = fake_copy(s, s.len);
	i = 0;
	while (i < (width & 7) && off + i < s.len)
	{
		p[off + i] = (uint8_t)(v >> (8 * i));
		i++;
	}
	if (!(width & 8))
		fake_seal(p, s.len);
	s.p = p;
	r = dex_open(&d, s);
	free(p);
	return (r);
}

uint32_t	fake_rand(uint32_t *state)
{
	*state ^= *state << 13;
	*state ^= *state >> 17;
	*state ^= *state << 5;
	return (*state);
}
