#include "harness.h"
#include "fake.h"
#include "dex_int.h"

static t_dex	g_d;

static uint32_t	code_of(uint32_t def_idx, uint32_t method_idx)
{
	t_dexclass	c;
	t_dexcdata	it;
	t_dexmember	m;

	if (dex_class(&g_d, def_idx, &c) != E_OK)
		return (0);
	if (dex_cdata_open(&g_d, &c, &it) != E_OK)
		return (0);
	while (dex_cdata_next(&it, &m) == E_OK)
		if (m.kind >= DEX_M_DIRECT && m.idx == method_idx)
			return (m.code_off);
	return (0);
}

static void	code(void)
{
	t_dexcode	c;
	uint32_t	end;

	end = (uint32_t)g_d.len & ~3u;
	h_eq_i64("code de add", dex_code(&g_d, code_of(1, 3), &c), E_OK);
	h_eq_u64("registres", c.registers, 3);
	h_eq_u64("entrees", c.ins, 2);
	h_eq_u64("sorties", c.outs, 0);
	h_eq_u64("blocs try", c.tries, 3);
	h_eq_u64("unites", c.insns_size, 5);
	h_eq_u64("premiere unite", dex_u16(c.insns), 0x0090);
	h_eq_u64("derniere unite", dex_u16(c.insns + 8), 0x000f);
	h_eq_u64("decalage", c.off, code_of(1, 3));
	h_eq_i64("decalage nul", dex_code(&g_d, 0, &c), E_INVAL);
	h_eq_i64("non aligne", dex_code(&g_d, code_of(1, 3) + 2, &c), E_INVAL);
	h_eq_i64("hors fichier", dex_code(&g_d, end, &c), E_INVAL);
	h_eq_i64("code simple", dex_code(&g_d, code_of(0, 1), &c), E_OK);
	h_eq_i64("sans try", dex_try_find(&g_d, &c, 0), E_NOENT);
}

static void	tries(void)
{
	t_dexcode	c;
	t_dexhit	it;
	t_dexcatch	k;
	t_dexstr	s;

	dex_code(&g_d, code_of(1, 3), &c);
	h_eq_i64("pc 0", dex_try_find(&g_d, &c, 0), 0);
	h_eq_i64("pc 1", dex_try_find(&g_d, &c, 1), 0);
	h_eq_i64("pc 2", dex_try_find(&g_d, &c, 2), 1);
	h_eq_i64("pc 3", dex_try_find(&g_d, &c, 3), 2);
	h_eq_i64("pc 4 non couvert", dex_try_find(&g_d, &c, 4), E_NOENT);
	h_eq_i64("try 0", dex_handler_open(&g_d, &c, 0, &it), E_OK);
	h_eq_i64("type", dex_handler_next(&it, &k), E_OK);
	dex_type(&g_d, k.type_idx, &s);
	h_eq_str("Exception", s.p, "Ljava/lang/Exception;");
	h_eq_u64("adresse 3", k.addr, 3);
	h_eq_i64("tout attraper", dex_handler_next(&it, &k), E_OK);
	h_true(k.type_idx == DEX_NO_INDEX && k.addr == 4, "tout en 4");
	h_eq_i64("fin", dex_handler_next(&it, &k), E_NOENT);
}

static void	tries_more(void)
{
	t_dexcode	c;
	t_dexhit	it;
	t_dexcatch	k;

	dex_code(&g_d, code_of(1, 3), &c);
	h_eq_i64("try 1", dex_handler_open(&g_d, &c, 1, &it), E_OK);
	h_eq_i64("type seul", dex_handler_next(&it, &k), E_OK);
	h_true(k.type_idx != DEX_NO_INDEX && k.addr == 3, "type en 3");
	h_eq_i64("fin sans tout attraper", dex_handler_next(&it, &k), E_NOENT);
	h_eq_i64("try 2", dex_handler_open(&g_d, &c, 2, &it), E_OK);
	h_eq_i64("tout attraper seul", dex_handler_next(&it, &k), E_OK);
	h_true(k.type_idx == DEX_NO_INDEX && k.addr == 4, "tout en 4");
	h_eq_i64("fin", dex_handler_next(&it, &k), E_NOENT);
	h_eq_i64("try 3 absent", dex_handler_open(&g_d, &c, 3, &it), E_RANGE);
	c.insns_size = 4;
	dex_handler_open(&g_d, &c, 2, &it);
	h_eq_i64("adresse hors code", dex_handler_next(&it, &k), E_INVAL);
	h_eq_i64("arret apres refus", dex_handler_next(&it, &k), E_NOENT);
}

int	main(void)
{
	h_begin("d03/code");
	h_true(dex_open(&g_d, fake_fixture()) == E_OK, "fixture ouverte");
	h_run("code_item", code);
	h_run("blocs try et gestionnaires", tries);
	h_run("gestionnaires : autres formes", tries_more);
	return (h_end());
}
