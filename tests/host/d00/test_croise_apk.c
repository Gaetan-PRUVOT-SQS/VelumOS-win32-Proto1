#include "harness.h"
#include "d00.h"

static void	check_manifest(t_axml *x)
{
	t_axmlattr	a;
	char		text[D00_TEXT];

	h_eq_i64("attribut package", d00_attr_named(x, "package", &a), 0);
	axml_string(x, a.raw, (t_text){text, sizeof(text)});
	h_eq_str("nom du paquet", text, "com.velum.bonjour");
	h_eq_i64("versionCode", axml_attr_find(x, AXML_ATTR_VERSION_CODE, &a), 0);
	h_eq_u64("versionCode vaut 1", a.data, 1);
}

static void	check_application(t_axml *x, const t_arsc *arsc)
{
	t_axmlattr	a;
	t_resquery	q;
	char		text[D00_TEXT];

	h_eq_i64("attribut label", axml_attr_find(x, AXML_ATTR_LABEL, &a), 0);
	h_eq_u64("label par reference", a.type, RES_T_REFERENCE);
	q = (t_resquery){a.data, "fr"};
	h_true(arsc_string(arsc, &q, (t_text){text, sizeof(text)}) >= 0,
		"libelle resolu");
	h_eq_str("libelle", text, "Bonjour APK");
}

static void	check_activity(t_axml *x)
{
	t_axmlattr	a;
	char		text[D00_TEXT];

	h_eq_i64("attribut name", axml_attr_find(x, AXML_ATTR_NAME, &a), 0);
	axml_string(x, a.raw, (t_text){text, sizeof(text)});
	h_eq_str("classe de l'activite", text, "com.velum.bonjour.Principale");
}

static void	apk_relu_par_les_lecteurs_c(void)
{
	t_d00apk	apk;
	t_axml		x;
	t_arsc		arsc;
	char		name[D00_TEXT];
	int			ev;

	h_eq_i64("archive et trois entrees", d00_apk_open(&apk, "bonjour.apk"), 0);
	h_eq_i64("axml_open", axml_open(&x, apk.manifest), 0);
	h_eq_i64("arsc_open", arsc_open(&arsc, apk.arsc), 0);
	ev = axml_next(&x);
	while (ev > 0)
	{
		if (ev == AXML_START && axml_name(&x, (t_text){name, sizeof(name)}) >= 0
			&& d00_same(name, "manifest"))
			check_manifest(&x);
		if (ev == AXML_START && d00_same(name, "application"))
			check_application(&x, &arsc);
		if (ev == AXML_START && d00_same(name, "activity"))
			check_activity(&x);
		ev = axml_next(&x);
	}
	h_eq_i64("fin du document sans erreur", ev, AXML_DONE);
	d00_apk_close(&apk);
}

int	main(void)
{
	h_begin("d00/croise-apk");
	h_run("apk_relu_par_les_lecteurs_c", apk_relu_par_les_lecteurs_c);
	return (h_end());
}
