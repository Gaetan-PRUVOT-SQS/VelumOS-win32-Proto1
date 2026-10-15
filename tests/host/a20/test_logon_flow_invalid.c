#include <stdint.h>
#include "harness.h"
#include "a20_test.h"
#include "auth.h"
#include "logon_flow.h"

static void	invalid_arret_hors_etat(void)
{
	t_logonflow	f;

	lf_init(&f);
	h_eq_i64("confirmer sans demande", lf_confirm_shutdown(&f), LA_NONE);
	lf_select(&f, 0, 1);
	lf_result(&f, AUTH_OK, 0);
	lf_ask_shutdown(&f);
	h_eq_i64("pas d'arret en session", f.mode, LM_SESSION);
	h_eq_i64("pas de confirmation en session", lf_confirm_shutdown(&f),
		LA_NONE);
	lf_cancel(&f);
	h_eq_i64("annuler en session sans effet", f.mode, LM_SESSION);
}

static void	invalid_choix_et_resultat_en_arret(void)
{
	t_logonflow	f;

	lf_init(&f);
	lf_ask_shutdown(&f);
	h_eq_i64("choix refuse pendant la confirmation", lf_select(&f, 1, 0),
		LA_NONE);
	h_eq_i64("mode intact", f.mode, LM_SHUTDOWN);
	h_eq_i64("resultat ignore pendant la confirmation",
		lf_result(&f, AUTH_OK, 0), LA_NONE);
	lf_ask_shutdown(&f);
	lf_cancel(&f);
	h_eq_i64("deux demandes puis annulation", f.mode, LM_PICK);
}

static void	invalid_fin_de_session(void)
{
	t_logonflow	f;

	lf_init(&f);
	lf_select(&f, 2, 1);
	lf_session_ended(&f);
	h_eq_i64("fin de session sans session", f.mode, LM_PASSWORD);
	lf_result(&f, AUTH_OK, 0);
	lf_session_ended(&f);
	h_eq_i64("retour au choix", f.mode, LM_PICK);
	h_eq_u64("compte oublie", f.selected, LF_NO_ACCOUNT);
	h_eq_i64("message efface", f.msg, LMSG_NONE);
	h_eq_i64("on peut rechoisir", lf_select(&f, 1, 1), LA_NONE);
	h_eq_i64("et retaper un mot de passe", f.mode, LM_PASSWORD);
}

int	main(void)
{
	h_begin("a20/logon-flow-invalid");
	h_run("flow invalide: arret hors etat", invalid_arret_hors_etat);
	h_run("flow invalide: choix en arret", invalid_choix_et_resultat_en_arret);
	h_run("flow invalide: fin de session", invalid_fin_de_session);
	return (h_end());
}
