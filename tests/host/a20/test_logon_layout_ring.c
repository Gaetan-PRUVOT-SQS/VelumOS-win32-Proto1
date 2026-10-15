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

static void	layout_puces_du_bas_de_la_maquette(void)
{
	t_logonlayout	o;

	logon_layout(1280, 800, 1, &o);
	h_true(rect_is(o.chips, lay_rect(28, 728, 1224, 44)),
		"puces a 28 des bords, hauteur 44");
	logon_layout(1024, 768, 1, &o);
	h_true(rect_is(o.chips, lay_rect(28, 696, 968, 44)), "puces a 1024x768");
}

static void	layout_boutons_rectangle_exterieur(void)
{
	t_logonlayout	o;

	logon_layout(1280, 800, 1, &o);
	h_eq_i64("anneau de focus de lib/ctl", LUNA_FOCUS_MARGIN, 4);
	h_true(rect_is(o.dlg_ok, lay_rect(152, 94, 104, 36)), "OK 96x28 + anneau");
	h_true(rect_is(o.dlg_cancel, lay_rect(256, 94, 104, 36)),
		"Annuler 96x28 + anneau");
	h_true(rect_is(o.tile[0], lay_rect(20, 24, 340, 138)), "tuile unique");
}

int	main(void)
{
	h_begin("a20/logon-layout-ring");
	h_run("layout: puces du bas", layout_puces_du_bas_de_la_maquette);
	h_run("layout: boutons exterieurs", layout_boutons_rectangle_exterieur);
	return (h_end());
}
