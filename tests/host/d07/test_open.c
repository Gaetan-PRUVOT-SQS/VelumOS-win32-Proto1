#include "harness.h"
#include "d07.h"

static const t_d07case	g_open[] = {
{"natif.apk", E_NOTSUP, APKR_NATIF},
{"multidex.apk", E_NOTSUP, APKR_MULTIDEX},
{"nom.apk", E_INVAL, APKR_NOM},
{"sans_dex.apk", E_NOENT, APKR_MANQUE},
{"sans_manifeste.apk", E_NOENT, APKR_MANQUE},
};

static void	open_table_refus(void)
{
	d07_table(g_open, sizeof(g_open) / sizeof(g_open[0]));
}

static void	open_limites(void)
{
	t_apk	a;
	uint8_t	octet;
	t_span	f;

	octet = 0;
	h_eq_i64("fichier nul", apk_open(&a, (t_span){NULL, 0}), E_INVAL);
	h_eq_i64("raison archive", a.reason, APKR_ARCHIVE);
	h_eq_i64("au-dessus du plafond", apk_open(&a,
			(t_span){&octet, (size_t)APK_FILE_MAX + 1}), E_RANGE);
	h_eq_i64("raison taille", a.reason, APKR_TAILLE);
	h_eq_i64("un octet", apk_open(&a, (t_span){&octet, 1}), E_INVAL);
	h_eq_i64("structure nulle", apk_open(NULL, (t_span){&octet, 1}), E_INVAL);
	f = d07_load("nosig.apk");
	h_eq_i64("non signe : ouverture permise", apk_open(&a, f), 0);
	h_eq_i64("raison ok", a.reason, APKR_OK);
	d07_free(f);
}

static void	reason_phrases(void)
{
	int	i;

	i = 0;
	while (i < APKR_COUNT)
	{
		h_true(apk_reason(i) != NULL && apk_reason(i)[0] != '\0',
			"phrase presente");
		i++;
	}
	h_eq_str("hors table", apk_reason(APKR_COUNT), "Refus inconnu.");
	h_eq_str("negatif", apk_reason(-1), "Refus inconnu.");
}

int	main(void)
{
	h_begin("d07/open");
	h_run("open_table_refus", open_table_refus);
	h_run("open_limites", open_limites);
	h_run("reason_phrases", reason_phrases);
	return (h_end());
}
