#include "dex_int.h"

static int	tries_area(const t_dex *d, t_dexcode *out, uint64_t end)
{
	out->tries_off = 0;
	out->handlers_off = 0;
	if (out->tries == 0)
		return (E_OK);
	end = (end + 3) & ~(uint64_t)3;
	if (!dex_fits(d, end, out->tries, 8)
		|| !dex_fits(d, end + 8 * (uint64_t)out->tries, 1, 1))
		return (E_INVAL);
	out->tries_off = (uint32_t)end;
	out->handlers_off = (uint32_t)(end + 8 * (uint64_t)out->tries);
	return (E_OK);
}

int	dex_code(const t_dex *d, uint32_t code_off, t_dexcode *out)
{
	const uint8_t	*p;
	uint64_t		end;

	if (!d || !out)
		return (E_INVAL);
	if (code_off < 0x70 || (code_off & 3) || !dex_fits(d, code_off, 16, 1))
		return (E_INVAL);
	p = d->p + code_off;
	out->off = code_off;
	out->registers = dex_u16(p);
	out->ins = dex_u16(p + 2);
	out->outs = dex_u16(p + 4);
	out->tries = dex_u16(p + 6);
	out->insns_size = dex_u32(p + 12);
	out->insns = p + 16;
	if (out->ins > out->registers
		|| !dex_fits(d, (uint64_t)code_off + 16, out->insns_size, 2))
		return (E_INVAL);
	end = (uint64_t)code_off + 16 + 2 * (uint64_t)out->insns_size;
	return (tries_area(d, out, end));
}

int	dex_try_find(const t_dex *d, const t_dexcode *code, uint32_t pc)
{
	uint32_t		i;
	const uint8_t	*t;

	if (!d || !code)
		return (E_INVAL);
	if (code->tries > 0xffff || !dex_fits(d, code->tries_off, code->tries, 8))
		return (E_INVAL);
	i = 0;
	while (i < code->tries)
	{
		t = d->p + code->tries_off + 8 * (size_t)i;
		if (pc >= dex_u32(t) && pc - dex_u32(t) < dex_u16(t + 4))
			return ((int)i);
		i++;
	}
	return (E_NOENT);
}
