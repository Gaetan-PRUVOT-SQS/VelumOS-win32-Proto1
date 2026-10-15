#include <stdint.h>
#include "harness.h"
#include "a20_test.h"
#include "auth.h"
#include "logon_flow.h"

static void	flow_depart_et_selection(void)
{
	t_logonflow	f;

	lf_init(&f);
	h_eq_i64("mode initial", f.mode, LM_PICK);
	h_eq_u64("rien de choisi", f.selected, LF_NO_ACCOUNT);
	h_eq_i64("choix avec mot de passe", lf_select(&f, 2, 1), LA_NONE);
	h_eq_i64("mode mot de passe", f.mode, LM_PASSWORD);
	h_eq_u64("compte choisi", f.selected, 2);
	h_eq_i64("second choix ignore", lf_select(&f, 5, 1), LA_NONE);
	h_eq_u64("compte inchange", f.selected, 2);
	lf_init(&f);
	h_eq_i64("choix sans mot de passe", lf_select(&f, 3, 0), LA_TRY_EMPTY);
	h_eq_i64("reste a l'ecran de choix", f.mode, LM_PICK);
	h_eq_u64("compte memorise", f.selected, 3);
}

static void	flow_resultats_d_authentification(void)
{
	t_logonflow	f;

	lf_init(&f);
	lf_select(&f, 1, 1);
	lf_result(&f, AUTH_BAD, 0);
	h_eq_i64("echec: message", f.msg, LMSG_BAD);
	h_eq_i64("echec: reste en saisie", f.mode, LM_PASSWORD);
	lf_result(&f, AUTH_WAIT, 7);
	h_eq_i64("attente: message", f.msg, LMSG_WAIT);
	h_eq_u64("attente: secondes", f.wait_s, 7);
	h_eq_i64("succes", lf_result(&f, AUTH_OK, 0), LA_START_SESSION);
	h_eq_i64("session ouverte", f.mode, LM_SESSION);
	h_eq_i64("message efface", f.msg, LMSG_NONE);
	h_eq_i64("resultat en session ignore", lf_result(&f, AUTH_OK, 0), LA_NONE);
	lf_init(&f);
	h_eq_i64("resultat sans compte ignore", lf_result(&f, AUTH_OK, 0), LA_NONE);
	h_eq_i64("mode intact", f.mode, LM_PICK);
}

static void	flow_refus_et_essai_a_vide(void)
{
	t_logonflow	f;

	lf_init(&f);
	lf_select(&f, 0, 1);
	lf_result(&f, AUTH_DENIED, 0);
	h_eq_i64("refus: retour au choix", f.mode, LM_PICK);
	h_eq_i64("refus: message d'erreur", f.msg, LMSG_ERROR);
	h_eq_u64("refus: plus de compte", f.selected, LF_NO_ACCOUNT);
	lf_select(&f, 0, 0);
	h_eq_i64("essai a vide reussi", lf_result(&f, AUTH_OK, 0),
		LA_START_SESSION);
	lf_init(&f);
	lf_select(&f, 0, 0);
	lf_result(&f, AUTH_BAD, 0);
	h_eq_i64("essai a vide refuse: demande le mot de passe", f.mode,
		LM_PASSWORD);
}

static void	flow_annulation_arret_fin_de_session(void)
{
	t_logonflow	f;

	lf_init(&f);
	lf_select(&f, 1, 1);
	lf_cancel(&f);
	h_eq_i64("annuler la saisie", f.mode, LM_PICK);
	h_eq_u64("compte oublie", f.selected, LF_NO_ACCOUNT);
	lf_ask_shutdown(&f);
	h_eq_i64("confirmation", f.mode, LM_SHUTDOWN);
	lf_cancel(&f);
	h_eq_i64("annuler l'arret depuis le choix", f.mode, LM_PICK);
	lf_select(&f, 1, 1);
	lf_ask_shutdown(&f);
	lf_cancel(&f);
	h_eq_i64("annuler l'arret depuis la saisie", f.mode, LM_PASSWORD);
	lf_ask_shutdown(&f);
	h_eq_i64("arret confirme", lf_confirm_shutdown(&f), LA_POWER_OFF);
}

int	main(void)
{
	h_begin("a20/logon-flow");
	h_run("flow: depart et selection", flow_depart_et_selection);
	h_run("flow: resultats d'authentification",
		flow_resultats_d_authentification);
	h_run("flow: refus et essai a vide", flow_refus_et_essai_a_vide);
	h_run("flow: annulation, arret", flow_annulation_arret_fin_de_session);
	return (h_end());
}
