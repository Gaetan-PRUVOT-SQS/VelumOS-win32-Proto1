#include "harness.h"
#include "velum/notif.h"

#define S 1000000000ull

static void	badge(void)
{
	char	b[NOTIF_BADGE_MAX];

	h_eq_i64("0 : vide", notif_badge_text(0, b, sizeof(b)), 0);
	h_eq_str("0", b, "");
	h_eq_i64("3 : un chiffre", notif_badge_text(3, b, sizeof(b)), 1);
	h_eq_str("3", b, "3");
	notif_badge_text(99, b, sizeof(b));
	h_eq_str("99", b, "99");
	h_eq_i64("100 : plafonne", notif_badge_text(100, b, sizeof(b)), 3);
	h_eq_str("100", b, "99+");
	notif_badge_text(4000000000u, b, sizeof(b));
	h_eq_str("grand", b, "99+");
	h_eq_i64("tampon nul", notif_badge_text(5, b, 0), 0);
	notif_badge_text(99, b, 2);
	h_eq_str("tampon de 2 : tronque", b, "9");
}

static void	age(void)
{
	char	t[NOTIF_AGE_MAX];

	notif_age_text(100 * S + 14 * S, 100 * S, t, sizeof(t));
	h_eq_str("maquette : 14 s", t, "il y a 14 s");
	notif_age_text(2 * 60 * S, 0, t, sizeof(t));
	h_eq_str("maquette : 2 min", t, "il y a 2 min");
	notif_age_text(59 * S, 0, t, sizeof(t));
	h_eq_str("59 s", t, "il y a 59 s");
	notif_age_text(60 * S, 0, t, sizeof(t));
	h_eq_str("60 s = 1 min", t, "il y a 1 min");
	notif_age_text(3600 * S, 0, t, sizeof(t));
	h_eq_str("1 h", t, "il y a 1 h");
	notif_age_text(86400 * S, 0, t, sizeof(t));
	h_eq_str("1 j", t, "il y a 1 j");
	notif_age_text(S - 1, 0, t, sizeof(t));
	h_eq_str("moins d'une seconde", t, "à l'instant");
	notif_age_text(5, 10, t, sizeof(t));
	h_eq_str("horloge reculee", t, "à l'instant");
}

int	main(void)
{
	h_begin("e13/texte");
	h_run("pastille : nombre reel, plafond 99+", badge);
	h_run("horodatage relatif", age);
	return (h_end());
}
