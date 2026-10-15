#include "dex_int.h"

static int	start(t_dexhit *it, const t_dexcur *c, int32_t n)
{
	int64_t	count;

	count = n;
	if (n < 0)
		count = -count;
	if (c->err || (uint64_t)count > c->len)
		return (E_INVAL);
	it->catch_all = (n <= 0);
	it->left = (uint32_t)count;
	it->pos = (uint32_t)c->pos;
	return (E_OK);
}

int	dex_handler_open(const t_dex *d, const t_dexcode *code,
		uint32_t try_index, t_dexhit *it)
{
	t_dexcur	c;
	uint64_t	at;

	if (!d || !code || !it)
		return (E_INVAL);
	it->d = d;
	it->left = 0;
	it->catch_all = 0;
	it->pos = 0;
	it->insns_size = code->insns_size;
	if (try_index >= code->tries)
		return (E_RANGE);
	if (code->tries > 0xffff || !dex_fits(d, code->tries_off, code->tries, 8))
		return (E_INVAL);
	at = code->tries_off + 8 * (uint64_t)try_index + 6;
	c = dex_cur(d, (uint64_t)code->handlers_off + dex_u16(d->p + at));
	return (start(it, &c, dex_sleb(&c)));
}

int	dex_handler_next(t_dexhit *it, t_dexcatch *out)
{
	t_dexcur	c;

	if (!it || !out)
		return (E_INVAL);
	if (it->left == 0 && !it->catch_all)
		return (E_NOENT);
	c = dex_cur(it->d, it->pos);
	out->type_idx = DEX_NO_INDEX;
	if (it->left > 0)
		out->type_idx = dex_uleb(&c);
	out->addr = dex_uleb(&c);
	if (c.err || out->addr >= it->insns_size
		|| (it->left > 0 && out->type_idx >= it->d->n[DEX_T_TYPE]))
	{
		it->left = 0;
		it->catch_all = 0;
		return (E_INVAL);
	}
	if (it->left > 0)
		it->left--;
	else
		it->catch_all = 0;
	it->pos = (uint32_t)c.pos;
	return (E_OK);
}
