#include "harness.h"
#include "fx.h"

static int	str(const t_arsc *a, uint32_t id, const char *lang, char *buf)
{
	t_resquery	q;
	t_text		t;

	q.res_id = id;
	q.lang = lang;
	t.p = buf;
	t.cap = 64;
	buf[0] = '\0';
	return (arsc_string(a, &q, t));
}

static void	chaines(void)
{
	t_span	s;
	t_arsc	a;
	char	buf[64];

	s = fx_load("resources.arsc");
	h_eq_i64("ouverture", arsc_open(&a, s), 0);
	h_eq_u64("un paquet", a.packages, 1);
	h_eq_i64("defaut", str(&a, 0x7f020000, NULL, buf), 5);
	h_eq_str("defaut texte", buf, "Hello");
	h_eq_i64("fr", str(&a, 0x7f020000, "fr", buf), 10);
	h_eq_str("fr texte", buf, "Bonjour \xc3\xa9");
	h_eq_i64("langue absente", str(&a, 0x7f020000, "de", buf), 5);
	h_eq_i64("reference", str(&a, 0x7f020001, "fr", buf), 10);
	h_eq_str("reference texte", buf, "Bonjour \xc3\xa9");
	h_eq_i64("cycle", str(&a, 0x7f020002, NULL, buf), E_RANGE);
	h_eq_i64("8 niveaux", str(&a, 0x7f020006, NULL, buf), 5);
	h_eq_i64("9 niveaux", str(&a, 0x7f02000e, NULL, buf), E_RANGE);
	h_eq_i64("entier", str(&a, 0x7f020004, NULL, buf), E_INVAL);
	h_eq_i64("complexe", str(&a, 0x7f020005, NULL, buf), E_NOTSUP);
	h_eq_i64("entree absente", str(&a, 0x7f020040, NULL, buf), E_NOENT);
	h_eq_i64("type absent", str(&a, 0x7f050000, NULL, buf), E_NOENT);
	h_eq_i64("paquet absent", str(&a, 0x01010001, NULL, buf), E_NOENT);
	fx_free(s);
}

static void	valeurs(void)
{
	t_span		s;
	t_arsc		a;
	t_resvalue	v;

	s = fx_load("resources.arsc");
	h_eq_i64("ouverture", arsc_open(&a, s), 0);
	h_eq_i64("entier", arsc_lookup(&a, 0x7f020004, "fr", &v), 0);
	h_eq_u64("type entier", v.type, RES_T_INT_DEC);
	h_eq_u64("valeur entier", v.data, 42);
	h_eq_i64("chaine fr", arsc_lookup(&a, 0x7f020000, "fr", &v), 0);
	h_eq_u64("indice fr", v.data, 1);
	h_eq_i64("chaine defaut", arsc_lookup(&a, 0x7f020000, NULL, &v), 0);
	h_eq_u64("indice defaut", v.data, 0);
	h_eq_i64("sortie nulle", arsc_lookup(&a, 0x7f020000, NULL, NULL), E_INVAL);
	fx_free(s);
}

static void	refus_un(const char *name, int open, int want)
{
	t_span	s;
	t_arsc	a;
	char	buf[64];

	s = fx_load(name);
	h_true(s.len > 0, name);
	h_eq_i64(name, arsc_open(&a, s), open);
	h_eq_i64(name, str(&a, 0x7f020000, NULL, buf), want);
	fx_free(s);
}

int	main(void)
{
	h_begin("d02 arsc");
	h_run("arsc_chaines", chaines);
	h_run("arsc_valeurs", valeurs);
	refus_un("arsc_nopool.arsc", E_INVAL, E_INVAL);
	refus_un("arsc_twopools.arsc", E_INVAL, E_INVAL);
	refus_un("arsc_badpkg.arsc", E_INVAL, E_INVAL);
	refus_un("arsc_sparse.arsc", 0, E_NOTSUP);
	refus_un("manifest.axml", E_INVAL, E_INVAL);
	return (h_end());
}
