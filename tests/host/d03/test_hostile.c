#include <stdlib.h>
#include "harness.h"
#include "fake.h"
#include "dex_int.h"

static t_dex	g_d;
static uint8_t	*g_p;

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

static void	hostile_code(void)
{
	t_dexcode	c;
	uint32_t	off;

	off = code_of(1, 3);
	h_true(off != 0, "code de add trouve");
	g_p[off + 15] = 0x7f;
	h_eq_i64("insns hors fichier", dex_code(&g_d, off, &c), E_INVAL);
	g_p[off + 15] = 0;
	g_p[off + 2] = 4;
	h_eq_i64("ins > registres", dex_code(&g_d, off, &c), E_INVAL);
	g_p[off + 2] = 2;
	g_p[off + 7] = 0xff;
	h_eq_i64("blocs try hors fichier", dex_code(&g_d, off, &c), E_INVAL);
	g_p[off + 7] = 0;
	h_eq_i64("code retabli", dex_code(&g_d, off, &c), E_OK);
	c.tries = 0x10000;
	h_eq_i64("t_dexcode forge", dex_try_find(&g_d, &c, 0), E_INVAL);
}

static void	hostile_handlers(void)
{
	t_dexcode	c;
	t_dexhit	it;
	t_dexcatch	k;

	dex_code(&g_d, code_of(1, 3), &c);
	g_p[c.tries_off + 6] = 0xff;
	g_p[c.tries_off + 7] = 0xff;
	h_eq_i64("gestionnaire hors fichier",
		dex_handler_open(&g_d, &c, 0, &it), E_INVAL);
	h_eq_i64("iterateur vide", dex_handler_next(&it, &k), E_NOENT);
	g_p[c.tries_off + 6] = 1;
	g_p[c.tries_off + 7] = 0;
	g_p[c.handlers_off + 2] = 0x7f;
	h_eq_i64("ouverture", dex_handler_open(&g_d, &c, 0, &it), E_OK);
	h_eq_i64("type hors table", dex_handler_next(&it, &k), E_INVAL);
	h_eq_i64("arret", dex_handler_next(&it, &k), E_NOENT);
}

static void	hostile_cdata(void)
{
	t_dexclass	c;
	t_dexcdata	it;
	t_dexmember	m;
	uint32_t	n;

	dex_class(&g_d, 1, &c);
	g_p[c.class_data_off + 4] = 0x7f;
	dex_cdata_open(&g_d, &c, &it);
	h_eq_i64("indice hors table", dex_cdata_next(&it, &m), E_INVAL);
	h_eq_i64("iteration arretee", dex_cdata_next(&it, &m), E_NOENT);
	g_p[c.class_data_off + 4] = 1;
	g_p[c.class_data_off + 6] = 9;
	dex_cdata_open(&g_d, &c, &it);
	h_eq_i64("premier membre", dex_cdata_next(&it, &m), E_OK);
	h_eq_i64("cumul hors table", dex_cdata_next(&it, &m), E_INVAL);
	g_p[c.class_data_off + 6] = 1;
	g_p[c.class_data_off] = 0x7f;
	dex_cdata_open(&g_d, &c, &it);
	n = 0;
	while (n < 1000 && dex_cdata_next(&it, &m) == E_OK)
		n++;
	h_true(n < 127, "compteur menteur : arret avant 127 membres");
	c.class_data_off = (uint32_t)g_d.len;
	h_eq_i64("hors fichier", dex_cdata_open(&g_d, &c, &it), E_INVAL);
}

int	main(void)
{
	t_span	s;

	h_begin("d03/hostile");
	s = fake_fixture();
	g_p = fake_copy(s, s.len);
	s.p = g_p;
	h_true(dex_open(&g_d, s) == E_OK, "copie ouverte");
	h_run("code_item hostile", hostile_code);
	h_run("gestionnaires hostiles", hostile_handlers);
	h_run("class_data hostile", hostile_cdata);
	free(g_p);
	return (h_end());
}
