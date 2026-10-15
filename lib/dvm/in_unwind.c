#include "in_int.h"

static int	in_match(t_in *in, const t_dexcatch *c)
{
	t_dref	before;
	int		rc;

	if (c->type_idx == DEX_NO_INDEX)
		return (1);
	before = in->vm->pending;
	rc = dvmrt_catches(in->vm, in->m, c->type_idx, before);
	if (rc > 0 && in->vm->pending != before)
		return (0);
	return (rc);
}

static int	in_handler(t_in *in, uint32_t *addr)
{
	t_dexcode	code;
	t_dexhit	it;
	t_dexcatch	c;
	int			rc;

	if (!in->m->tries || !in->m->dex)
		return (0);
	if (dex_code(in->m->dex, in->m->code_off, &code) < 0)
		return (0);
	rc = dex_try_find(in->m->dex, &code, in->pc);
	if (rc < 0 || dex_handler_open(in->m->dex, &code, (uint32_t)rc, &it) < 0)
		return (0);
	rc = 0;
	while (rc == 0 && dex_handler_next(&it, &c) == 0)
	{
		rc = in_match(in, &c);
		*addr = c.addr;
	}
	return (rc);
}

int	in_unwind(t_in *in)
{
	uint32_t	addr;
	int			rc;

	addr = 0;
	rc = in_handler(in, &addr);
	while (rc == 0 && !in->done)
	{
		in_pop(in);
		if (!in->done)
			rc = in_handler(in, &addr);
	}
	if (rc < 0)
		return (rc);
	if (in->done)
		return (DVM_THROWN);
	in->pc = addr;
	return (0);
}
