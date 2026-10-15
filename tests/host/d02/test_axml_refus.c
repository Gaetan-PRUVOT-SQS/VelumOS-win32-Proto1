#include "harness.h"
#include "fx.h"

static void	un(const char *name, int want)
{
	t_span	s;

	s = fx_load(name);
	h_true(s.len > 0, name);
	h_eq_i64(name, fx_walk_axml(s), want);
	fx_free(s);
}

static void	refus(void)
{
	un("bad_end.axml", E_INVAL);
	un("no_end.axml", E_INVAL);
	un("zero_chunk.axml", E_INVAL);
	un("bad_name.axml", E_INVAL);
	un("bad_end_name.axml", E_INVAL);
	un("attr_over.axml", E_INVAL);
	un("attr_small.axml", E_INVAL);
	un("text_bad.axml", E_INVAL);
	un("two_pools.axml", E_INVAL);
	un("two_maps.axml", E_INVAL);
	un("no_pool.axml", E_INVAL);
	un("chunk_over.axml", E_INVAL);
	un("short_node.axml", E_INVAL);
}

static void	attribut_un(const char *name)
{
	t_span		s;
	t_axml		x;
	t_axmlattr	a;

	s = fx_load(name);
	h_eq_i64("ouverture", axml_open(&x, s), 0);
	h_eq_i64("element", axml_next(&x), AXML_START);
	h_eq_i64(name, axml_attr(&x, 0, &a), E_INVAL);
	fx_free(s);
}

static void	limites(void)
{
	t_span	s;
	t_axml	x;

	un("deep_ok.axml", AXML_DONE);
	un("deep_ko.axml", E_RANGE);
	attribut_un("attr_name.axml");
	attribut_un("attr_raw.axml");
	attribut_un("attr_str.axml");
	s = fx_load("manifest.axml");
	fx_poke(s, 0, 2);
	h_eq_i64("type du document", axml_open(&x, s), E_INVAL);
	h_eq_i64("suite apres refus", axml_next(&x), E_INVAL);
	h_eq_i64("pointeur nul", axml_open(NULL, s), E_INVAL);
	fx_free(s);
	s = fx_load("bad_end.axml");
	h_eq_i64("ouverture", axml_open(&x, s), 0);
	h_eq_i64("erreur", axml_next(&x), E_INVAL);
	h_eq_i64("erreur tenace", axml_next(&x), E_INVAL);
	fx_free(s);
}

int	main(void)
{
	h_begin("d02 axml refus");
	h_run("axml_refus", refus);
	h_run("axml_limites", limites);
	return (h_end());
}
