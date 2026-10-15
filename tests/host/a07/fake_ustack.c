#include <stdlib.h>
#include <string.h>
#include "a07_fake.h"

static uint64_t	ust_word(const t_ustack *st, uint64_t va, int *ok)
{
	uint64_t	v;

	if (va < st->top - st->cap || va + 8 > st->top || (va & 7))
	{
		*ok = 0;
		return (0);
	}
	memcpy(&v, st->buf + st->cap - (st->top - va), 8);
	return (v);
}

const char	*ust_str(const t_ustack *st, uint64_t va)
{
	if (va < st->top - st->cap || va >= st->top)
		return ("");
	return ((const char *)st->buf + st->cap - (st->top - va));
}

static void	ust_aux(const t_ustack *st, uint64_t va, t_ustparsed *out)
{
	out->naux = 0;
	while (out->ok && out->naux < UST_MAX)
	{
		out->aux_type[out->naux] = ust_word(st, va, &out->ok);
		out->aux_val[out->naux] = ust_word(st, va + 8, &out->ok);
		va += 16;
		out->naux++;
		if (out->aux_type[out->naux - 1] == AT_NULL)
			return ;
	}
	out->ok = 0;
}

void	ust_parse(const t_ustack *st, t_ustparsed *out)
{
	uint64_t	k;

	memset(out, 0, sizeof(*out));
	out->ok = ((st->sp & 15) == 0);
	out->argc = ust_word(st, st->sp, &out->ok);
	k = 0;
	while (out->ok && k <= out->argc)
	{
		if (k < UST_MAX)
			out->argv[k] = ust_word(st, st->sp + 8 * (k + 1), &out->ok);
		k++;
	}
	if (ust_word(st, st->sp + 8 * (out->argc + 1), &out->ok) != 0)
		out->ok = 0;
	if (ust_word(st, st->sp + 8 * (out->argc + 2), &out->ok) != 0)
		out->ok = 0;
	ust_aux(st, st->sp + 8 * (out->argc + 3), out);
}

int	ust_build(t_ustack *st, uint64_t cap, const t_ustart *in)
{
	st->cap = cap;
	st->top = UST_TOP;
	st->buf = malloc(cap + (cap == 0));
	if (!st->buf)
		abort();
	return (ustack_build(st, in));
}
