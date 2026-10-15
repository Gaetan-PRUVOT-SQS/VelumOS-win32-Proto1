#include <string.h>
#include "harness.h"
#include "d09.h"

int	fk_units(const uint16_t *u, uint32_t n, const t_case *c, uint64_t *ret)
{
	t_dmethod	m;
	uint32_t	args[4];
	uint32_t	k;

	memset(&m, 0, sizeof(m));
	k = 0;
	while (k < n && k < 32)
	{
		g_fk.code[2 * k] = (uint8_t)u[k];
		g_fk.code[2 * k + 1] = (uint8_t)(u[k] >> 8);
		k++;
	}
	m.insns = (t_span){g_fk.code, 2 * (size_t)k};
	m.registers = 6;
	m.ins = 4;
	args[0] = (uint32_t)c->x;
	args[1] = (uint32_t)(c->x >> 32);
	args[2] = (uint32_t)c->y;
	args[3] = (uint32_t)(c->y >> 32);
	return (dvm_call(&g_fk.vm, &m, args, ret));
}

void	fk_play(const uint16_t *u, uint32_t n, const t_case *c)
{
	uint64_t	ret;
	int			rc;

	fk_init(NULL, 8, 0);
	ret = 0;
	rc = fk_units(u, n, c, &ret);
	h_eq_i64(c->what, rc, c->rc);
	if (rc == 0)
		h_eq_u64(c->what, ret, c->want);
	fk_check(c->what);
	fk_end();
}

int	fk_calc(const char *cls, const char *name, uint64_t *ret)
{
	const t_dmethod	*m;

	m = fk_method(cls, name);
	h_true(m != NULL, name);
	if (!m)
		return (E_NOENT);
	return (dvm_call(&g_fk.vm, m, g_fk.args, ret));
}
