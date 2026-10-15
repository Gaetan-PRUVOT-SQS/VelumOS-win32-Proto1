#include <stdint.h>
#include "harness.h"
#include "velum/libk.h"
#include "a20_test.h"
#include "layout.h"
#include "logon_layout.h"

static int	rect_is(t_rect r, t_rect e)
{
	return (r.x == e.x && r.y == e.y && r.w == e.w && r.h == e.h);
}

static void	layout_valeurs_de_la_maquette_1280x800(void)
{
	t_logonlayout	o;

	h_eq_i64("un compte", logon_layout(1280, 800, 1, &o), 1);
	h_true(rect_is(o.clock, lay_rect(0, 100, 1280, 120)), "horloge");
	h_true(rect_is(o.date, lay_rect(0, 226, 1280, 28)),
		"date a 6 sous l'horloge");
	h_true(rect_is(o.card, lay_rect(450, 311, 380, 290)), "carte 380");
	h_true(rect_is(o.avatar, lay_rect(146, 28, 88, 88)), "avatar 88 centre");
	h_true(rect_is(o.name, lay_rect(24, 132, 332, 26)), "nom sous l'avatar");
	h_true(rect_is(o.field, lay_rect(24, 174, 332, 48)), "champ pilule 48");
	h_true(rect_is(o.password, lay_rect(42, 174, 266, 48)), "saisie a 18");
	h_true(rect_is(o.go, lay_rect(309, 175, 46, 46)),
		"bouton rond 38 + anneau");
	h_true(rect_is(o.message, lay_rect(24, 238, 228, 28)), "message");
	h_true(rect_is(o.shutdown, lay_rect(256, 234, 104, 36)),
		"Eteindre + anneau");
}

static void	layout_deux_comptes_en_liste(void)
{
	t_logonlayout	o;

	h_eq_i64("deux", logon_layout(1024, 768, 2, &o), 2);
	h_true(rect_is(o.tile[0], lay_rect(20, 24, 340, 64)),
		"premiere ligne + anneau");
	h_true(rect_is(o.tile[1], lay_rect(20, 88, 340, 64)),
		"seconde ligne + anneau");
	h_true(rect_is(o.field, lay_rect(24, 164, 332, 48)), "champ sous la liste");
}

static void	layout_horloge_masquee_quand_la_hauteur_manque(void)
{
	t_logonlayout	o;

	logon_layout(800, 600, 1, &o);
	h_eq_i64("horloge masquee a 600", o.clock.h, 0);
	logon_layout(800, 700, 1, &o);
	h_eq_i64("horloge visible a 700", o.clock.h, 120);
	h_true(o.clock.y >= 16 && o.clock.y <= 100, "haut entre 16 et 100");
}

int	main(void)
{
	h_begin("a20/logon-layout-mockup");
	h_run("layout: maquette 1280x800", layout_valeurs_de_la_maquette_1280x800);
	h_run("layout: deux comptes", layout_deux_comptes_en_liste);
	h_run("layout: horloge masquee",
		layout_horloge_masquee_quand_la_hauteur_manque);
	return (h_end());
}
