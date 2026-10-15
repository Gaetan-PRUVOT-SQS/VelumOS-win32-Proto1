#include "in_int.h"

static void	in_load(t_in *in, uint32_t fp)
{
	uint32_t	*h;

	h = in->vm->stack + fp;
	__builtin_memcpy(&in->m, h, sizeof(in->m));
	in->fp = fp;
	in->r = h + IN_HDR;
	in->nreg = in->m->registers;
}

static void	in_fill(uint32_t *h, const t_dmethod *m, const uint32_t *args)
{
	uint32_t	k;
	uint32_t	first;

	h[0] = 0;
	h[1] = 0;
	__builtin_memcpy(h, &m, sizeof(m));
	h[2] = 0;
	first = m->registers - m->ins;
	k = 0;
	while (k < m->registers)
	{
		h[IN_HDR + k] = 0;
		if (k >= first)
			h[IN_HDR + k] = args[k - first];
		k++;
	}
}

int	in_push(t_in *in, const t_dmethod *m, const uint32_t *args)
{
	t_dvm		*vm;
	uint32_t	need;
	uint32_t	fp;

	vm = in->vm;
	need = IN_HDR + m->registers;
	if (m->ins > m->registers || (m->ins && !args))
		return (in_throw(in, IN_VERIFY));
	if (vm->depth >= vm->lim.depth_max || vm->sp > vm->stack_words
		|| need > vm->stack_words - vm->sp)
		return (in_throw(in, IN_SOE));
	fp = vm->sp;
	if (in->m)
		vm->stack[in->fp + 2] = in->pc;
	in_fill(vm->stack + fp, m, args);
	vm->stack[fp + 3] = in->fp;
	vm->sp += need;
	vm->depth++;
	in_load(in, fp);
	in->pc = 0;
	in->next = 0;
	return (0);
}

void	in_pop(t_in *in)
{
	uint32_t	prev;

	prev = in->vm->stack[in->fp + 3];
	in->vm->sp = in->fp;
	in->vm->depth--;
	if (in->fp == in->fp0)
	{
		in->done = 1;
		return ;
	}
	in_load(in, prev);
	in->pc = in->vm->stack[prev + 2];
	in->next = in->pc + IN_INVOKE_LEN;
}

int	in_native(t_in *in, const t_dmethod *m, const uint32_t *args)
{
	uint64_t	ret;
	int			rc;

	if (in->vm->depth >= in->vm->lim.depth_max)
		return (in_throw(in, IN_SOE));
	ret = 0;
	in->vm->depth++;
	rc = m->native(in->vm, args, &ret);
	in->vm->depth--;
	if (rc == 0)
		in->vm->result = ret;
	return (rc);
}
