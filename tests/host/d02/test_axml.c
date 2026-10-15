#include <string.h>
#include "harness.h"
#include "fx.h"

static void	expect(t_axml *x, int ev, const char *name)
{
	char	buf[64];
	t_text	t;

	t.p = buf;
	t.cap = sizeof(buf);
	h_eq_i64("evenement", axml_next(x), ev);
	buf[0] = '\0';
	if (ev == AXML_TEXT)
		axml_text(x, t);
	else if (ev != AXML_DONE)
		axml_name(x, t);
	h_eq_str("nom", buf, name);
}

static void	attr_is(const t_axml *x, uint32_t id, uint32_t type, uint32_t data)
{
	t_axmlattr	a;

	memset(&a, 0, sizeof(a));
	h_eq_i64("attribut trouve", axml_attr_find(x, id, &a), 0);
	h_eq_u64("type", a.type, type);
	h_eq_u64("valeur", a.data, data);
}

static void	manifeste(const char *name)
{
	t_span	s;
	t_axml	x;

	s = fx_load(name);
	h_eq_i64("ouverture", axml_open(&x, s), 0);
	expect(&x, AXML_START, "manifest");
	h_eq_u64("3 attributs", axml_attr_count(&x), 3);
	attr_is(&x, AXML_ATTR_VERSION_CODE, RES_T_INT_DEC, 1);
	attr_is(&x, AXML_ATTR_VERSION_NAME, RES_T_STRING, 13);
	expect(&x, AXML_START, "uses-sdk");
	attr_is(&x, AXML_ATTR_MIN_SDK, RES_T_INT_DEC, 21);
	attr_is(&x, AXML_ATTR_TARGET_SDK, RES_T_INT_DEC, 33);
	expect(&x, AXML_END, "uses-sdk");
	expect(&x, AXML_START, "application");
	attr_is(&x, AXML_ATTR_LABEL, RES_T_REFERENCE, 0x7f020000);
	attr_is(&x, AXML_ATTR_DEBUGGABLE, RES_T_INT_BOOLEAN, 0xffffffff);
	expect(&x, AXML_START, "activity");
	attr_is(&x, AXML_ATTR_NAME, RES_T_STRING, 17);
	expect(&x, AXML_TEXT, "texte \xc3\xa9\xf0\x9f\x98\x80");
	expect(&x, AXML_END, "activity");
	expect(&x, AXML_END, "application");
	expect(&x, AXML_END, "manifest");
	expect(&x, AXML_DONE, "");
	expect(&x, AXML_DONE, "");
	fx_free(s);
}

static void	attributs(void)
{
	t_span		s;
	t_axml		x;
	t_axmlattr	a;
	char		buf[64];
	t_text		t;

	s = fx_load("manifest.axml");
	t.p = buf;
	t.cap = sizeof(buf);
	h_eq_i64("ouverture", axml_open(&x, s), 0);
	h_eq_u64("aucun attribut avant", axml_attr_count(&x), 0);
	h_eq_i64("attribut avant", axml_attr(&x, 0, &a), E_INVAL);
	h_eq_i64("nom avant", axml_name(&x, t), E_INVAL);
	h_eq_i64("premier", axml_next(&x), AXML_START);
	h_eq_i64("attribut sans identifiant", axml_attr(&x, 2, &a), 0);
	h_eq_u64("res_id nul", a.res_id, 0);
	h_eq_i64("nom attribut", axml_string(&x, a.name, t), 7);
	h_eq_str("package", buf, "package");
	h_eq_i64("valeur brute", axml_string(&x, a.raw, t), 17);
	h_eq_str("paquet", buf, "org.velum.bonjour");
	h_eq_i64("indice hors liste", axml_attr(&x, 3, &a), E_RANGE);
	h_eq_i64("absent", axml_attr_find(&x, AXML_ATTR_ICON, &a), E_NOENT);
	h_eq_i64("identifiant nul", axml_attr_find(&x, 0, &a), E_INVAL);
	h_eq_i64("texte hors texte", axml_text(&x, t), E_INVAL);
	fx_free(s);
}

int	main(void)
{
	h_begin("d02 axml");
	manifeste("manifest.axml");
	manifeste("manifest_u8.axml");
	h_run("axml_attributs", attributs);
	return (h_end());
}
