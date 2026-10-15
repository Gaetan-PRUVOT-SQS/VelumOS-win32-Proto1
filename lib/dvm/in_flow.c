#include "in_int.h"

int	in_op_goto(t_in *in)
{
	in->next = in->pc + (uint32_t)in->i.lit;
	return (0);
}

static uint32_t	in_sparse(const t_dswitch *sw, int32_t key)
{
	uint32_t	k;

	k = 0;
	while (k < sw->count && dexcode_rd32(sw->keys + 4 * (size_t)k) != key)
		k++;
	return (k);
}

int	in_op_switch(t_in *in)
{
	t_dswitch	sw;
	int32_t		key;
	uint32_t	k;

	if (dexcode_switch(in->m->insns, in->pc + (uint32_t)in->i.lit, &sw) < 0)
		return (E_INVAL);
	key = (int32_t)in_get(in, in->i.a);
	k = (uint32_t)key - (uint32_t)sw.first_key;
	if (sw.sparse)
		k = in_sparse(&sw, key);
	if (k < sw.count)
		in->next = in->pc
			+ (uint32_t)dexcode_rd32(sw.targets + 4 * (size_t)k);
	return (0);
}

static int	in_cond(uint32_t k, int32_t x, int32_t y)
{
	if (k == 0)
		return (x == y);
	if (k == 1)
		return (x != y);
	if (k == 2)
		return (x < y);
	if (k == 3)
		return (x >= y);
	if (k == 4)
		return (x > y);
	return (x <= y);
}

int	in_op_if(t_in *in)
{
	int32_t		y;
	uint32_t	k;

	y = 0;
	k = in->i.op - OP_IF_EQZ;
	if (in->i.op <= OP_IF_LE)
	{
		k = in->i.op - OP_IF_EQ;
		y = (int32_t)in_get(in, in->i.b);
	}
	if (in_cond(k, (int32_t)in_get(in, in->i.a), y))
		in->next = in->pc + (uint32_t)in->i.lit;
	return (0);
}
