#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "help.h"

uint32_t	hr_drain(t_handle c, uint32_t type, void *last)
{
	uint64_t	buf[WM_MSG_MAX / 8];
	t_wmhdr		h;
	t_handle	hx;
	uint32_t	n;

	n = 0;
	while (fk_recv(c, buf, sizeof(buf), &hx) >= 0)
	{
		memcpy(&h, buf, sizeof(h));
		if (hx != 0)
			fk_close(hx);
		if (type == 0 || h.type == type)
		{
			n++;
			if (last != NULL)
				memcpy(last, buf, h.size);
		}
	}
	return (n);
}

void	hr_arg(t_wmarg *m, uint32_t type, uint32_t window, uint32_t value)
{
	memset(m, 0, sizeof(*m));
	m->h.magic = WM_MAGIC;
	m->h.type = type;
	m->h.size = sizeof(*m);
	m->h.window = window;
	m->value = value;
}

uint64_t	hg_next(uint64_t *st)
{
	uint64_t	x;

	x = *st;
	x ^= x >> 12;
	x ^= x << 25;
	x ^= x >> 27;
	*st = x;
	return (x * 0x2545f4914f6cdd1dull);
}

uint32_t	hg_below(uint64_t *st, uint32_t n)
{
	if (n == 0)
		return (0);
	return ((uint32_t)(hg_next(st) % n));
}

uint64_t	hg_seed(const char *suite)
{
	const char	*env;
	uint64_t	seed;

	env = getenv("A18_SEED");
	seed = 0x5eed1234abcdull;
	if (env != NULL)
		seed = strtoull(env, NULL, 0);
	if (seed == 0)
		seed = 1;
	printf("%s : graine 0x%llx\n", suite, (unsigned long long)seed);
	return (seed);
}
