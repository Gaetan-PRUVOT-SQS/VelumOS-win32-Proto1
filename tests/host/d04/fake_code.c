#include <string.h>
#include "fake.h"

t_code	g_c;

void	fake_reset(void)
{
	memset(g_c.b, 0, sizeof(g_c.b));
	g_c.n = 0;
}

void	fake_u(uint32_t unit)
{
	if (g_c.n >= sizeof(g_c.b) / 2)
		return ;
	g_c.b[2 * g_c.n] = (uint8_t)(unit & 0xff);
	g_c.b[2 * g_c.n + 1] = (uint8_t)((unit >> 8) & 0xff);
	g_c.n++;
}

void	fake_us(const uint16_t *u, uint32_t n)
{
	uint32_t	i;

	fake_reset();
	i = 0;
	while (i < n)
		fake_u(u[i++]);
}

int	fake_dec_r(uint64_t lo, uint32_t hi)
{
	t_dinsn	in;
	t_span	s;

	fake_reset();
	fake_u((uint32_t)(lo & 0xffff));
	fake_u((uint32_t)((lo >> 16) & 0xffff));
	fake_u((uint32_t)((lo >> 32) & 0xffff));
	fake_u((uint32_t)((lo >> 48) & 0xffff));
	fake_u(hi);
	s.p = g_c.b;
	s.len = 10;
	return (dexcode_decode(s, 0, &in));
}

t_dinsn	fake_dec(uint64_t lo, uint32_t hi)
{
	t_dinsn	in;
	t_span	s;

	memset(&in, 0xa5, sizeof(in));
	fake_dec_r(lo, hi);
	s.p = g_c.b;
	s.len = 10;
	dexcode_decode(s, 0, &in);
	return (in);
}
