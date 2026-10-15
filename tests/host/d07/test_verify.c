#include "harness.h"
#include "d07.h"

static const t_d07case	g_accepte[] = {
{"ok.apk", 0, APKR_OK},
{"refait.apk", 0, APKR_OK},
{"paire_inconnue.apk", 0, APKR_OK},
{"commentaire.apk", 0, APKR_OK},
};

static const t_d07case	g_refuse[] = {
{"nosig.apk", E_ACCES, APKR_SIGNATURE},
{"zip_simple.apk", E_ACCES, APKR_SIGNATURE},
{"alt_entree.apk", E_ACCES, APKR_SIGNATURE},
{"alt_repertoire.apk", E_ACCES, APKR_SIGNATURE},
{"alt_fin.apk", D07_ANY, 0},
{"alt_bloc.apk", E_ACCES, APKR_SIGNATURE},
{"algo_0104.apk", E_NOTSUP, APKR_ALGO},
{"algo_inconnu.apk", E_ACCES, APKR_SIGNATURE},
{"condensat_faux.apk", E_ACCES, APKR_SIGNATURE},
{"cle_autre.apk", E_ACCES, APKR_SIGNATURE},
{"deux_signataires.apk", E_NOTSUP, APKR_SIGNATAIRES},
{"zero_signataire.apk", E_ACCES, APKR_SIGNATURE},
{"decale.apk", E_ACCES, APKR_SIGNATURE},
{"commentaire_magie.apk", E_ACCES, APKR_SIGNATURE},
{"long_0.apk", E_ACCES, APKR_SIGNATURE},
{"long_1.apk", E_ACCES, APKR_SIGNATURE},
{"long_2.apk", E_ACCES, APKR_SIGNATURE},
{"long_3.apk", E_ACCES, APKR_SIGNATURE},
{"long_4.apk", E_ACCES, APKR_SIGNATURE},
{"tailles.apk", E_ACCES, APKR_SIGNATURE},
};

static void	verify_table_acceptes(void)
{
	d07_table(g_accepte, sizeof(g_accepte) / sizeof(g_accepte[0]));
}

static void	verify_table_refus(void)
{
	d07_table(g_refuse, sizeof(g_refuse) / sizeof(g_refuse[0]));
}

static void	verify_identite_du_signataire(void)
{
	t_span		f;
	t_apk		a;
	t_apksig	s;
	t_apksig	t;

	f = d07_load("ok.apk");
	h_eq_i64("ouverture", apk_open(&a, f), 0);
	h_eq_i64("signature", apk_verify(&a, &s), 0);
	h_eq_u64("algorithme", s.algo, APK_ALGO_RSA_PKCS1_SHA256);
	h_true((s.cert_sha256[0] | s.cert_sha256[1] | s.cert_sha256[2]) != 0,
		"empreinte du certificat remplie");
	a.reason = APKR_NATIF;
	h_eq_i64("archive refusee jamais verifiee", apk_verify(&a, &t), E_ACCES);
	h_eq_u64("empreinte a zero au refus", t.cert_sha256[0], 0);
	h_eq_i64("sortie nulle", apk_verify(&a, NULL), E_INVAL);
	d07_free(f);
}

int	main(void)
{
	h_begin("d07/verify");
	h_run("verify_table_acceptes", verify_table_acceptes);
	h_run("verify_table_refus", verify_table_refus);
	h_run("verify_identite_du_signataire", verify_identite_du_signataire);
	return (h_end());
}
