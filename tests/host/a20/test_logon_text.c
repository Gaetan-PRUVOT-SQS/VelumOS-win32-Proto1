#include <stdint.h>
#include "harness.h"
#include "velum/libk.h"
#include "a20_test.h"
#include "logon_text.h"
#include "typo.h"

static void	text_messages_par_etat(void)
{
	t_logonflow	f;
	char		out[LOGON_MSG_MAX];

	lf_init(&f);
	h_eq_i64("aucun message", logon_message(&f, 0, out, sizeof(out)), 0);
	h_eq_str("vide", out, "");
	f.msg = LMSG_BAD;
	logon_message(&f, 0, out, sizeof(out));
	h_eq_str("echec", out, "Échec de la connexion. Réessayez.");
	logon_message(&f, 7, out, sizeof(out));
	h_eq_str("echec et attente", out,
		"Échec de la connexion. Patientez 7" NBSP "s");
	f.msg = LMSG_WAIT;
	f.wait_s = 30;
	logon_message(&f, 0, out, sizeof(out));
	h_eq_str("attente", out, "Trop d'échecs. Patientez 30" NBSP "s");
	f.msg = LMSG_ERROR;
	logon_message(&f, 0, out, sizeof(out));
	h_eq_str("erreur", out, "Ce compte n'est pas disponible.");
	f.msg = LMSG_POWER;
	logon_message(&f, 0, out, sizeof(out));
	h_eq_str("arret impossible", out, "Impossible d'éteindre l'ordinateur.");
	f.msg = LMSG_EMPTY;
	logon_message(&f, 0, out, sizeof(out));
	h_eq_str("aucun compte", out, "Aucun compte utilisateur n'est disponible.");
}

static void	text_ne_revele_pas_la_cause(void)
{
	t_logonflow	f;
	char		out[LOGON_MSG_MAX];

	lf_init(&f);
	f.msg = LMSG_BAD;
	logon_message(&f, 3, out, sizeof(out));
	h_true(strstr(out, "incorrect") == NULL, "ne dit pas incorrect");
	h_true(strstr(out, "inconnu") == NULL, "ne dit pas inconnu");
	h_true(strstr(out, "existe") == NULL, "ne parle pas d'existence");
	f.msg = LMSG_WAIT;
	f.wait_s = UINT32_MAX;
	logon_message(&f, 0, out, sizeof(out));
	h_true(strstr(out, "4294967295" NBSP "s") != NULL, "grande attente rendue");
}

static void	text_tampon_trop_petit(void)
{
	t_logonflow	f;
	char		out[8];

	lf_init(&f);
	f.msg = LMSG_BAD;
	h_eq_i64("tronque", logon_message(&f, 0, out, sizeof(out)), -34);
	h_eq_u64("termine", strlen(out), sizeof(out) - 1);
	h_eq_i64("taille nulle", logon_message(&f, 0, out, 0), -34);
	f.msg = LMSG_NONE;
	h_eq_i64("rien a ecrire, taille 1", logon_message(&f, 0, out, 1), 0);
}

int	main(void)
{
	h_begin("a20/logon-text");
	h_run("texte: messages par etat", text_messages_par_etat);
	h_run("texte: ne revele pas la cause", text_ne_revele_pas_la_cause);
	h_run("texte: tampon trop petit", text_tampon_trop_petit);
	return (h_end());
}
