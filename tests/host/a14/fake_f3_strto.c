#include <stdio.h>
#include "errno.h"
#include "fake_f3.h"
#include "harness.h"
#include "stdlib.h"

static int	f3_want_errno(int w)
{
	if (w)
		return (w);
	return (F3_SENTINEL);
}

static void	f3_check_strtol(const t_f3_strtol *c, size_t idx, int ll)
{
	char		*end;
	long long	got;
	int			ok;

	errno = F3_SENTINEL;
	end = NULL;
	if (ll)
		got = strtoll(c->in, &end, c->base);
	else
		got = strtol(c->in, &end, c->base);
	ok = got == c->want && end == c->in + c->end_off
		&& errno == f3_want_errno(c->want_errno);
	h_true(ok, "strtol : valeur, fin et errno");
	if (!ok)
		fprintf(stderr, "  cas %zu '%s' base %d : %lld fin %ld errno %d\n", idx,
			c->in, c->base, got, (long)(end - c->in), errno);
}

void	f3_strtol_run(const t_f3_strtol *c, size_t n, int ll)
{
	size_t	i;

	i = 0;
	while (i < n)
	{
		f3_check_strtol(&c[i], i, ll);
		i++;
	}
}

static void	f3_check_strtoul(const t_f3_strtoul *c, size_t idx, int ull)
{
	char				*end;
	unsigned long long	got;
	int					ok;

	errno = F3_SENTINEL;
	end = NULL;
	if (ull)
		got = strtoull(c->in, &end, c->base);
	else
		got = strtoul(c->in, &end, c->base);
	ok = got == c->want && end == c->in + c->end_off
		&& errno == f3_want_errno(c->want_errno);
	h_true(ok, "strtoul : valeur, fin et errno");
	if (!ok)
		fprintf(stderr, "  cas %zu '%s' base %d : %llu fin %ld errno %d\n", idx,
			c->in, c->base, got, (long)(end - c->in), errno);
}

void	f3_strtoul_run(const t_f3_strtoul *c, size_t n, int ull)
{
	size_t	i;

	i = 0;
	while (i < n)
	{
		f3_check_strtoul(&c[i], i, ull);
		i++;
	}
}
