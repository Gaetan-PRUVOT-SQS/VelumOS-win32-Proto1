#include "proc_pure.h"
#include "velum/err.h"
#include "velum/libk.h"
#include "velum/util.h"

int	ustack_push(t_ustack *st, const void *src, uint64_t n)
{
	uint64_t	used;

	used = st->top - st->sp;
	if (used > st->cap || n > st->cap - used)
		return (E_NOMEM);
	if (n == 0)
		return (0);
	st->sp -= n;
	memcpy(st->buf + st->cap - (used + n), src, n);
	return (0);
}

int	ustack_put(t_ustack *st, uint64_t va, uint64_t val)
{
	uint64_t	off;

	if (va < st->top - st->cap || va > st->top - sizeof(val) || (va & 7))
		return (E_INVAL);
	off = st->cap - (st->top - va);
	memcpy(st->buf + off, &val, sizeof(val));
	return (0);
}

int	ustack_align(t_ustack *st, uint64_t n)
{
	uint64_t	used;
	uint64_t	sp;

	used = st->top - st->sp;
	if (used > st->cap || n > st->cap - used)
		return (E_NOMEM);
	sp = align_down(st->sp - n, 16);
	if (st->top - sp > st->cap)
		return (E_NOMEM);
	st->sp = sp;
	return (0);
}

static int	ustack_strings(t_ustack *st, const t_ustart *in)
{
	int	rc;

	rc = ustack_push(st, in->args, in->args_len);
	if (rc == 0)
		rc = ustack_push(st, "", 1);
	if (rc == 0)
		rc = ustack_push(st, in->path, in->path_len);
	return (rc);
}

int	ustack_build(t_ustack *st, const t_ustart *in)
{
	uint64_t	str_va;
	uint64_t	rnd_va;
	int			rc;

	st->sp = st->top;
	if ((st->top & 15) || st->cap > st->top || in->nargs > PROC_NARGS_MAX)
		return (E_INVAL);
	rc = ustack_strings(st, in);
	str_va = st->sp;
	if (rc == 0)
		rc = ustack_align(st, 0);
	if (rc == 0)
		rc = ustack_push(st, in->rnd, USTACK_RANDOM);
	rnd_va = st->sp;
	if (rc == 0)
		rc = ustack_vectors(st, in, str_va, rnd_va);
	return (rc);
}
