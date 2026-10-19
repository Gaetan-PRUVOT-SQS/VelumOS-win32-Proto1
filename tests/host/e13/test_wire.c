#include <string.h>
#include "harness.h"
#include "velum/err.h"
#include "velum/notif.h"

static void	round_trip(void)
{
	t_notifmsg	m;
	t_notifmsg	out;

	h_eq_i64("construction", notif_msg_post(&m, "Bonjour", "Session",
			"Session ouverte pour Utilisateur.", 7), 0);
	h_eq_i64("decodage", notif_msg_decode(&m, sizeof(m), &out), 0);
	h_eq_str("titre", out.title, "Session");
	h_eq_i64("icone", out.icon, 7);
}

static void	length_and_header(void)
{
	t_notifmsg	m;
	t_notifmsg	out;

	notif_msg_post(&m, "A", "T", "x", 0);
	h_eq_i64("trop court", notif_msg_decode(&m, sizeof(m) - 1, &out), E_INVAL);
	h_eq_i64("trop long", notif_msg_decode(&m, sizeof(m) + 1, &out), E_INVAL);
	m.magic ^= 1;
	h_eq_i64("magie", notif_msg_decode(&m, sizeof(m), &out), E_INVAL);
	m.magic ^= 1;
	m.size = 1;
	h_eq_i64("taille annoncee", notif_msg_decode(&m, sizeof(m), &out), E_INVAL);
	h_eq_i64("NULL", notif_msg_decode(NULL, sizeof(m), &out), E_INVAL);
}

static void	unknown_type(void)
{
	t_notifmsg	m;
	t_notifmsg	out;

	notif_msg_init(&m, 0);
	h_eq_i64("type 0", notif_msg_decode(&m, sizeof(m), &out), E_INVAL);
	m.type = NOTIFC_CLEAR + 1;
	h_eq_i64("type inconnu", notif_msg_decode(&m, sizeof(m), &out), E_INVAL);
	notif_msg_init(&m, NOTIFC_COUNT);
	h_eq_i64("type connu", notif_msg_decode(&m, sizeof(m), &out), 0);
	notif_msg_init(&m, NOTIFC_DISMISS);
	h_eq_i64("retrait sans id", notif_msg_decode(&m, sizeof(m), &out), E_INVAL);
}

static void	unterminated_fields(void)
{
	t_notifmsg	m;
	t_notifmsg	out;

	notif_msg_post(&m, "A", "T", "x", 0);
	memset(m.title, 'a', sizeof(m.title));
	h_eq_i64("titre sans NUL", notif_msg_decode(&m, sizeof(m), &out), E_INVAL);
	notif_msg_post(&m, "A", "T", "x", 0);
	memset(m.text, 'b', sizeof(m.text));
	h_eq_i64("texte sans NUL", notif_msg_decode(&m, sizeof(m), &out), E_INVAL);
	notif_msg_post(&m, "A", "T", "x", 0);
	m.app[0] = '\0';
	h_eq_i64("appli vide", notif_msg_decode(&m, sizeof(m), &out), E_INVAL);
}

static void	too_long_refused_by_builder(void)
{
	t_notifmsg	m;
	char		big[NOTIF_TITLE_MAX + 8];

	memset(big, 'z', sizeof(big) - 1);
	big[sizeof(big) - 1] = '\0';
	h_eq_i64("titre trop long", notif_msg_post(&m, "A", big, "x", 0), E_INVAL);
	h_eq_i64("titre NULL", notif_msg_post(&m, "A", NULL, "x", 0), E_INVAL);
}

int	main(void)
{
	h_begin("e13/wire");
	h_run("aller-retour d'un depot", round_trip);
	h_run("longueur et en-tete", length_and_header);
	h_run("types de requete", unknown_type);
	h_run("champs non termines refuses", unterminated_fields);
	h_run("construction : champ trop long refuse", too_long_refused_by_builder);
	return (h_end());
}
