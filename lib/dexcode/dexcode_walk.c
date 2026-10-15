#include "dexcode_int.h"

int	dexcode_bit(const uint8_t *starts, uint32_t pc)
{
	return ((starts[pc >> 3] >> (pc & 7)) & 1);
}

static void	bit_clear(uint8_t *starts, uint32_t pc)
{
	starts[pc >> 3] &= (uint8_t)(0xff ^ (1u << (pc & 7)));
}

int	dexcode_is_payload(const t_dscan *s, uint32_t pc)
{
	uint32_t	u;

	u = dexcode_unit(s->insns.p, pc);
	return ((u & 0xff) == 0 && (u >> 8) != 0);
}

uint32_t	dexcode_item_len(const t_dscan *s, uint32_t pc)
{
	t_dinsn		in;
	t_dpayload	pl;

	if (dexcode_is_payload(s, pc))
	{
		if (dexcode_payload(s->insns, pc, &pl) < 0)
			return (0);
		return ((uint32_t)pl.units);
	}
	if (dexcode_decode(s->insns, pc, &in) < 0)
		return (0);
	return (in.len);
}

void	dexcode_strip(t_dscan *s)
{
	uint32_t	pc;
	uint32_t	len;

	pc = 0;
	len = 1;
	while (pc < s->n && len != 0)
	{
		len = dexcode_item_len(s, pc);
		if (dexcode_is_payload(s, pc))
		{
			bit_clear(s->starts, pc);
			if (pc > 0 && dexcode_unit(s->insns.p, pc - 1) == 0
				&& dexcode_bit(s->starts, pc - 1))
				bit_clear(s->starts, pc - 1);
		}
		pc += len;
	}
}
