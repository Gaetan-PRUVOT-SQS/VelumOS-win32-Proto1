#include "proc_pure.h"
#include "velum/abi/abi_syscall.h"
#include "velum/err.h"

static uint64_t	ustack_next_string(const t_ustack *st, uint64_t str_va)
{
	uint64_t	off;

	if (str_va >= st->top || str_va < st->top - st->cap)
		return (0);
	off = st->cap - (st->top - str_va);
	while (off < st->cap && st->buf[off] != '\0')
		off++;
	if (off >= st->cap)
		return (0);
	return (st->top - (st->cap - off) + 1);
}

static int	ustack_argv(t_ustack *st, uint64_t va, uint64_t str_va,
				uint32_t argc)
{
	uint32_t	k;

	k = 0;
	while (k < argc)
	{
		if (ustack_put(st, va + 8 * ((uint64_t)k + 1), str_va) < 0)
			return (E_INVAL);
		str_va = ustack_next_string(st, str_va);
		if (str_va == 0)
			return (E_INVAL);
		k++;
	}
	if (ustack_put(st, va + 8 * ((uint64_t)argc + 1), 0) < 0)
		return (E_INVAL);
	if (ustack_put(st, va + 8 * ((uint64_t)argc + 2), 0) < 0)
		return (E_INVAL);
	return (0);
}

static int	aux_pair(t_ustack *st, uint64_t *va, uint64_t type, uint64_t val)
{
	if (ustack_put(st, *va, type) < 0 || ustack_put(st, *va + 8, val) < 0)
		return (E_INVAL);
	*va += 16;
	return (0);
}

static int	ustack_auxv(t_ustack *st, uint64_t va, const t_ustart *in,
				uint64_t rnd_va)
{
	int	rc;

	rc = aux_pair(st, &va, AT_PHDR, in->phdr);
	if (rc == 0)
		rc = aux_pair(st, &va, AT_PHNUM, in->phnum);
	if (rc == 0)
		rc = aux_pair(st, &va, AT_PAGESZ, 4096);
	if (rc == 0)
		rc = aux_pair(st, &va, AT_ENTRY, in->entry);
	if (rc == 0)
		rc = aux_pair(st, &va, AT_RANDOM, rnd_va);
	if (rc == 0)
		rc = aux_pair(st, &va, AT_VELUM_ABI, VELUM_ABI_VERSION);
	if (rc == 0)
		rc = aux_pair(st, &va, AT_VELUM_FLAGS, in->flags);
	if (rc == 0)
		rc = aux_pair(st, &va, AT_NULL, 0);
	return (rc);
}

int	ustack_vectors(t_ustack *st, const t_ustart *in, uint64_t str_va,
		uint64_t rnd_va)
{
	uint64_t	words;
	uint32_t	argc;
	int			rc;

	argc = in->nargs + 1;
	words = 1 + argc + 2 + 2 * USTACK_AUX_PAIRS;
	rc = ustack_align(st, words * sizeof(uint64_t));
	if (rc == 0)
		rc = ustack_put(st, st->sp, argc);
	if (rc == 0)
		rc = ustack_argv(st, st->sp, str_va, argc);
	if (rc == 0)
		rc = ustack_auxv(st, st->sp + 8 * ((uint64_t)argc + 3), in, rnd_va);
	return (rc);
}
