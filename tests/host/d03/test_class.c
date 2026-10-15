#include <stdlib.h>
#include "harness.h"
#include "fake.h"
#include "dex_int.h"

static const uint8_t	g_idx[12] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 2, 3, 4};
static const uint8_t	g_kind[12] = {0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 3};
static t_dex			g_d;

static void	classes(void)
{
	t_dexclass	c;
	t_dexstr	s;

	h_eq_i64("Derived", dex_find_class(&g_d, "Lvelum/Derived;"), 1);
	h_eq_i64("Base", dex_find_class(&g_d, "Lvelum/Base;"), 0);
	h_eq_i64("absente", dex_find_class(&g_d, "Lvelum/None;"), E_NOENT);
	h_eq_i64("lecture", dex_class(&g_d, 1, &c), E_OK);
	h_eq_u64("public final", c.access, ACC_PUBLIC | ACC_FINAL);
	dex_type(&g_d, c.superclass_idx, &s);
	h_eq_str("heritage", s.p, "Lvelum/Base;");
	h_eq_u64("une interface", c.interfaces.n, 1);
	dex_type(&g_d, dex_list_at(&c.interfaces, 0), &s);
	h_eq_str("interface", s.p, "Ljava/lang/Runnable;");
	h_true(fake_named(&g_d, c.source_file_idx, "Demo.java"), "source");
	h_eq_i64("Base", dex_class(&g_d, 0, &c), E_OK);
	h_eq_u64("sans interface", c.interfaces.n, 0);
	h_eq_i64("hors table", dex_class(&g_d, 2, &c), E_RANGE);
}

static void	cdata(void)
{
	t_dexclass	c;
	t_dexcdata	it;
	t_dexmember	m;
	uint32_t	i;

	dex_class(&g_d, 1, &c);
	h_eq_i64("ouverture", dex_cdata_open(&g_d, &c, &it), E_OK);
	i = 0;
	while (i < 12)
	{
		h_eq_i64("membre", dex_cdata_next(&it, &m), E_OK);
		h_eq_u64("genre", m.kind, g_kind[i]);
		h_eq_u64("indice cumule", m.idx, g_idx[i]);
		h_true((m.code_off != 0) == (m.kind >= DEX_M_DIRECT), "code");
		i++;
	}
	h_eq_u64("acces de la derniere", m.access, ACC_PUBLIC);
	h_eq_i64("fin", dex_cdata_next(&it, &m), E_NOENT);
	h_eq_i64("fin stable", dex_cdata_next(&it, &m), E_NOENT);
	c.class_data_off = 0;
	h_eq_i64("sans class_data", dex_cdata_open(&g_d, &c, &it), E_OK);
	h_eq_i64("aucun membre", dex_cdata_next(&it, &m), E_NOENT);
}

static void	values(void)
{
	t_dexclass	c;
	t_dexvalue	v;

	dex_class(&g_d, 1, &c);
	h_eq_i64("long", dex_static_value(&g_d, &c, 0, &v), E_OK);
	h_true(v.kind == DEX_V_LONG && v.i == -2, "BIG = -2");
	h_eq_i64("int", dex_static_value(&g_d, &c, 1, &v), E_OK);
	h_eq_i64("COUNT = 300", v.i, 300);
	h_eq_i64("booleen", dex_static_value(&g_d, &c, 2, &v), E_OK);
	h_eq_i64("FLAG = vrai", v.i, 1);
	h_eq_i64("type", dex_static_value(&g_d, &c, 3, &v), E_NOTSUP);
	h_eq_i64("chaine", dex_static_value(&g_d, &c, 4, &v), E_OK);
	h_true(v.kind == DEX_V_STRING && fake_named(&g_d, (uint32_t)v.i,
			"V\xc3\xa9lum \xe2\x82\xac \xed\xa0\xbd\xed\xb8\x80"), "NAME");
	h_eq_i64("nul", dex_static_value(&g_d, &c, 5, &v), E_OK);
	h_eq_u64("genre nul", v.kind, DEX_V_NULL);
	h_eq_i64("double", dex_static_value(&g_d, &c, 6, &v), E_OK);
	h_eq_u64("PI", (uint64_t)v.i, 0x400921fb54442d18);
	h_eq_i64("flottant", dex_static_value(&g_d, &c, 7, &v), E_OK);
	h_eq_u64("RATIO = 1.5", (uint64_t)v.i, 0x3fc00000);
	h_eq_i64("au-dela", dex_static_value(&g_d, &c, 8, &v), E_NOENT);
	dex_class(&g_d, 0, &c);
	h_eq_i64("Base", dex_static_value(&g_d, &c, 0, &v), E_OK);
	h_eq_i64("seed = -2 sur un octet", v.i, -2);
}

static void	hostile_values(void)
{
	t_dexclass	c;
	t_dexvalue	v;
	uint8_t		*p;
	t_dex		d;
	t_span		s;

	s = fake_fixture();
	p = fake_copy(s, s.len);
	s.p = p;
	dex_class(&g_d, 1, &c);
	p[c.static_values_off + 1] = 0x1c;
	fake_seal(p, s.len);
	h_eq_i64("ouverture", dex_open(&d, s), E_OK);
	h_eq_i64("tableau", dex_static_value(&d, &c, 1, &v), E_NOTSUP);
	p[c.static_values_off + 1] = 0x01;
	h_eq_i64("genre inconnu", dex_static_value(&d, &c, 0, &v), E_INVAL);
	p[c.static_values_off + 1] = 0xe4;
	h_eq_i64("int de 8 octets", dex_static_value(&d, &c, 0, &v), E_INVAL);
	p[c.static_values_off + 1] = 0x5f;
	h_eq_i64("booleen 2", dex_static_value(&d, &c, 0, &v), E_INVAL);
	p[c.static_values_off] = 0xff;
	h_eq_i64("taille LEB", dex_static_value(&d, &c, 0, &v), E_INVAL);
	free(p);
}

int	main(void)
{
	h_begin("d03/class");
	h_true(dex_open(&g_d, fake_fixture()) == E_OK, "fixture ouverte");
	h_run("classes, heritage, interface", classes);
	h_run("class_data : indices cumules", cdata);
	h_run("valeurs statiques", values);
	h_run("valeurs statiques hostiles", hostile_values);
	return (h_end());
}
