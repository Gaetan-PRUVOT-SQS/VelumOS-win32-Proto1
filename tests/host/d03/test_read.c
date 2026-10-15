#include "harness.h"
#include "fake.h"
#include "dex_int.h"

static t_dex	g_d;

static void	strings(void)
{
	t_dexstr	s;
	char		b[64];
	t_text		t;
	int			i;

	t.p = b;
	t.cap = sizeof(b);
	i = fake_string_idx(&g_d,
			"V\xc3\xa9lum \xe2\x82\xac \xed\xa0\xbd\xed\xb8\x80");
	h_true(i >= 0, "chaine non ASCII presente");
	h_eq_i64("lecture", dex_string(&g_d, (uint32_t)i, &s), E_OK);
	h_eq_u64("longueur UTF-16", s.utf16, 10);
	h_eq_u64("taille en octets", s.size, 17);
	h_eq_i64("conversion", mutf8_to_utf8(s.p, t), 15);
	h_eq_str("UTF-8", b, "V\xc3\xa9lum \xe2\x82\xac \xf0\x9f\x98\x80");
	i = fake_string_idx(&g_d, "a\xc0\x80" "b");
	h_true(i >= 0, "chaine avec U+0000 presente");
	dex_string(&g_d, (uint32_t)i, &s);
	h_eq_u64("trois unites", s.utf16, 3);
	h_eq_i64("U+0000 non convertible", mutf8_to_utf8(s.p, t), E_NOTSUP);
	h_eq_i64("hors table", dex_string(&g_d, g_d.n[0], &s), E_RANGE);
	h_eq_i64("sortie nulle", dex_string(&g_d, 0, 0), E_INVAL);
}

static void	protos(void)
{
	t_dexproto	p;
	t_dexstr	s;

	h_eq_i64("proto 0", dex_proto(&g_d, 0, &p), E_OK);
	h_eq_i64("shorty", dex_string(&g_d, p.shorty_idx, &s), E_OK);
	h_eq_str("shorty III", s.p, "III");
	h_eq_i64("retour", dex_type(&g_d, p.return_idx, &s), E_OK);
	h_eq_str("retour I", s.p, "I");
	h_eq_u64("deux parametres", p.params.n, 2);
	dex_type(&g_d, dex_list_at(&p.params, 1), &s);
	h_eq_str("parametre I", s.p, "I");
	h_eq_u64("hors liste", dex_list_at(&p.params, 2), DEX_NO_INDEX);
	h_eq_i64("proto 1", dex_proto(&g_d, 1, &p), E_OK);
	h_eq_u64("sans parametre", p.params.n, 0);
	h_eq_i64("hors table", dex_proto(&g_d, 2, &p), E_RANGE);
	h_eq_i64("type hors table", dex_type(&g_d, g_d.n[1], &s), E_RANGE);
}

static void	members(void)
{
	t_dexfield	f;
	t_dexmethod	m;
	t_dexstr	s;

	h_eq_i64("champ 2", dex_field(&g_d, 2, &f), E_OK);
	h_true(fake_named(&g_d, f.name_idx, "COUNT"), "nom COUNT");
	dex_type(&g_d, f.class_idx, &s);
	h_eq_str("classe du champ", s.p, "Lvelum/Derived;");
	dex_type(&g_d, f.type_idx, &s);
	h_eq_str("type du champ", s.p, "I");
	h_eq_i64("methode 3", dex_method(&g_d, 3, &m), E_OK);
	h_true(fake_named(&g_d, m.name_idx, "add"), "nom add");
	h_eq_u64("proto de add", m.proto_idx, 0);
	h_eq_i64("champ hors table", dex_field(&g_d, 10, &f), E_RANGE);
	h_eq_i64("methode hors table", dex_method(&g_d, 5, &m), E_RANGE);
}

int	main(void)
{
	h_begin("d03/read");
	h_true(dex_open(&g_d, fake_fixture()) == E_OK, "fixture ouverte");
	h_run("chaines non ASCII", strings);
	h_run("prototypes et types", protos);
	h_run("champs et methodes", members);
	return (h_end());
}
