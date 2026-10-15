#include <string.h>
#include "harness.h"
#include "help.h"

static t_wsrv	g_s;

static void	section_quota(void)
{
	t_handle	a;

	hs_start(&g_s, 1024, 768);
	a = hr_client(&g_s);
	while (hr_create(&g_s, a, rect_make(0, 0, 1024, 768), WS_DEFAULT) != 0)
		h_true(g_s.c[0].sec_bytes <= WM_SECTION_MAX, "plafond respecte");
	h_eq_i64("cinq fenetres plein ecran", g_s.t.nused, 5);
	h_eq_i64("plafond 16 Mio", g_hr.status, E_NOSPC);
	fk_close(a);
	hs_settle(&g_s, a);
	h_eq_i64("sections rendues", g_s.sec_total, 0);
	hs_stop(&g_s);
}

static void	section_faults(void)
{
	t_handle	a;
	uint32_t	live;

	hs_start(&g_s, 320, 240);
	a = hr_client(&g_s);
	live = fk_live();
	g_fk.fail_section = 0;
	h_eq_i64("section refusee", hr_create(&g_s, a, rect_make(0, 0, 200,
				200), WS_DEFAULT), 0);
	g_fk.fail_map = 0;
	h_eq_i64("mappage refuse", hr_create(&g_s, a, rect_make(0, 0, 200,
				200), WS_DEFAULT), 0);
	h_true(fk_live() == live && g_s.t.nused == 0, "aucune fuite");
	h_true(hr_create(&g_s, a, rect_make(0, 0, 200, 200), WS_DEFAULT) != 0,
		"retour a la normale");
	h_eq_i64("client garde", g_s.violations, 0);
	fk_close(a);
	hs_stop(&g_s);
	h_eq_i64("tout libere", fk_live(), 0);
}

int	main(void)
{
	h_begin("a18/srv_faults");
	h_run("plafond des sections", section_quota);
	h_run("pannes de section", section_faults);
	return (h_end());
}
