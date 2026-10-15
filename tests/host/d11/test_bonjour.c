#include "harness.h"
#include "d11.h"

#define PRINCIPALE "Lcom/velum/bonjour/Principale;"

static void	arbre_apres_demarrage(void)
{
	t_d11				t;
	const t_droidview	*v;

	h_eq_i64("ouverture", d11_open(&t, "bonjour.dex", 0), 0);
	h_eq_i64("droid_start", droid_start(&t.d, PRINCIPALE), 0);
	h_eq_str("aucune erreur", t.d.error, "");
	h_eq_u64("trois vues", t.d.nviews, 3);
	h_eq_u64("racine", t.d.root, 1);
	v = &t.d.views[0];
	h_eq_u64("racine LinearLayout", v->kind, DV_LINEAR);
	h_eq_u64("racine verticale", v->orientation, DROID_VERTICAL);
	h_eq_u64("deux enfants", v->nkids, 2);
	h_eq_u64("premier enfant", v->kids[0], 2);
	h_eq_u64("second enfant", v->kids[1], 3);
	h_eq_u64("TextView", t.d.views[1].kind, DV_TEXT);
	h_eq_str("texte", t.d.views[1].text, "Bonjour depuis un APK");
	h_eq_u64("Button", t.d.views[2].kind, DV_BUTTON);
	h_eq_str("bouton", t.d.views[2].text, "Compter");
	h_eq_u64("parent du bouton", t.d.views[2].parent, 1);
	h_eq_u64("une ligne de journal", t.nlogs, 1);
	h_eq_u64("niveau info", t.level, DROID_LOG_INFO);
	h_eq_str("ligne de Log.i", t.log, "Bonjour: onCreate");
	h_eq_u64("changement visible", t.d.dirty, 1);
	d11_close(&t);
}

static void	trois_clics_gc_et_arret(void)
{
	t_d11		t;
	uint32_t	i;

	h_eq_i64("ouverture", d11_open(&t, "bonjour.dex", 0), 0);
	h_eq_i64("droid_start", droid_start(&t.d, PRINCIPALE), 0);
	i = 0;
	while (i++ < 3)
	{
		t.d.dirty = 0;
		h_eq_i64("clic", droid_click(&t.d, 3), 0);
		h_eq_u64("clic visible", t.d.dirty, 1);
		dvm_gc(t.vm);
	}
	h_eq_str("compteur", t.d.views[1].text, "Clics : 3");
	h_eq_str("bouton intact", t.d.views[2].text, "Compter");
	h_eq_i64("clic sans ecouteur", droid_click(&t.d, 2), 0);
	h_eq_i64("clic sur vue inconnue", droid_click(&t.d, 9), E_INVAL);
	h_eq_i64("clic sur vue zero", droid_click(&t.d, 0), E_INVAL);
	h_eq_i64("droid_stop", droid_stop(&t.d), 0);
	h_eq_u64("vues rendues", t.d.nviews, 0);
	h_eq_u64("activite rendue", t.d.activity, DVM_NULL);
	h_eq_i64("clic apres arret", droid_click(&t.d, 3), E_INVAL);
	h_eq_i64("second arret", droid_stop(&t.d), E_INVAL);
	d11_close(&t);
}

static void	dix_mille_clics_tas_un_mio(void)
{
	t_d11			t;
	t_dheapstats	st;
	uint32_t		i;
	uint32_t		bad;
	uint32_t		base;

	h_eq_i64("ouverture", d11_open(&t, "bonjour.dex", 1048576), 0);
	h_eq_i64("droid_start", droid_start(&t.d, PRINCIPALE), 0);
	h_eq_i64("premier clic", droid_click(&t.d, 3), 0);
	dvm_gc(t.vm);
	dvm_heap_stats(t.vm, &st);
	base = st.bytes;
	i = 1;
	bad = 0;
	while (i++ < 10000)
		bad += (droid_click(&t.d, 3) != 0);
	h_eq_u64("aucun clic en echec", bad, 0);
	h_eq_str("compteur", t.d.views[1].text, "Clics : 10000");
	dvm_gc(t.vm);
	dvm_heap_stats(t.vm, &st);
	h_eq_u64("tas sans croissance", st.bytes, base);
	h_true(st.peak_bytes <= 1048576, "pic sous le plafond");
	h_true(st.collections > 10, "collectes declenchees par le plafond");
	d11_close(&t);
}

int	main(void)
{
	h_begin("d11/bonjour");
	h_run("arbre_apres_demarrage", arbre_apres_demarrage);
	h_run("trois_clics_gc_et_arret", trois_clics_gc_et_arret);
	h_run("dix_mille_clics_tas_un_mio", dix_mille_clics_tas_un_mio);
	return (h_end());
}
