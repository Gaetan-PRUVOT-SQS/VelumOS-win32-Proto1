#include "fake_f4.h"
#include "velum/err.h"

static size_t	f4_run(const uint64_t *w, size_t n)
{
	size_t	len;

	len = 0;
	while (len < n && w[len])
		len++;
	return (len);
}

static int	f4_model_aux(const uint64_t *w, size_t from, size_t n)
{
	size_t	pair;

	pair = 0;
	while (pair < START_AUX_MAX && from + 2 * pair < n)
	{
		if (w[from + 2 * pair] == AT_NULL)
			return (START_OK);
		if (from + 2 * pair + 1 >= n)
			break ;
		pair++;
	}
	return (START_TRUNCATED);
}

int	f4_model(const uint64_t *w, size_t n)
{
	size_t	argc;
	size_t	len;

	if (!w || n < 3)
		return (E_INVAL);
	if (w[0] > START_ARGC_MAX)
		return (E_RANGE);
	argc = (size_t)w[0];
	if (argc + 2 > n || f4_run(w + 1, argc) != argc || w[1 + argc] != 0)
		return (E_PROTO);
	len = f4_run(w + 2 + argc, n - 2 - argc);
	if (len > START_ENVC_MAX)
		return (E_INVAL);
	if (2 + argc + len >= n)
		return (E_PROTO);
	return (f4_model_aux(w, 3 + argc + len, n));
}

uint64_t	f4_rng(uint64_t *state)
{
	uint64_t	x;

	x = *state;
	x ^= x << 13;
	x ^= x >> 7;
	x ^= x << 17;
	*state = x;
	return (x);
}

uint64_t	f4_u(const void *p)
{
	return ((uint64_t)(uintptr_t)p);
}
