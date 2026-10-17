#include "harness.h"
#include "render.h"
#include "logon_paint.h"

static void	logon_mesures_de_la_maquette(void)
{
	t_logon	lg;

	render_reset(A20_FIXTURE);
	logon_boot(&lg);
	h_eq_i64("avatar de 88", lg.lay.avatar.w, 88);
	h_eq_i64("champ pilule de 48", lg.lay.field.h, 48);
	h_eq_i64("bouton rond de 38 dans son anneau", lg.lay.go.w - 2 * 4, 38);
	h_eq_i64("rectangle exterieur du bouton rond", lg.lay.go.w, 46);
	h_eq_i64("anneau de focus de lib/ctl", LUNA_FOCUS_MARGIN, 4);
	h_eq_i64("haut de l'horloge a y 100", lg.lay.clock.y, 100);
	h_eq_i64("horloge de 120", lg.lay.clock.h, 120);
	uiwin_close(&lg.ui);
}

static void	logon_date_depuis_les_secondes(void)
{
	char	clock[LOGON_HEAD_TEXT];
	char	date[LOGON_HEAD_TEXT];

	logon_clock_text(0, clock, date, sizeof(date));
	h_eq_str("origine des temps", date, "jeudi 1 janvier");
	logon_clock_text(951782400ull * 1000000000ull, clock, date,
		sizeof(date));
	h_eq_str("29 fevrier 2000", date, "mardi 29 février");
	logon_clock_text(WALL_NS, clock, date, sizeof(date));
	h_eq_str("date de la fixture", date, "lundi 5 octobre");
	h_eq_i64("deux points de l'horloge", clock[2], ':');
}

int	main(void)
{
	h_begin("a20/render-logon-metrics");
	h_run("rendu logon: mesures de la maquette", logon_mesures_de_la_maquette);
	h_run("rendu logon: date civile", logon_date_depuis_les_secondes);
	return (h_end());
}
