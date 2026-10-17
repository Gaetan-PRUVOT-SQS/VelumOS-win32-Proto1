#include <stdlib.h>
#include "harness.h"
#include "help.h"

static t_wsrv	g_s;

static int	pixels_differents(void)
{
	t_surface	ref;
	uint32_t	*px;
	int			bad;

	px = calloc(320 * 240, 4);
	if (px == NULL)
		return (-1);
	gfx_surface_init(&ref, px, 320, 240);
	bad = hx_compare(&g_s, &ref);
	free(px);
	return (bad);
}

static void	start_one(int *slot)
{
	t_handle	c;
	uint32_t	id;

	*slot = hs_one(&g_s, &c, &id);
	ws_mark(&g_s, g_s.dirty.screen);
	ws_frame(&g_s);
}

static void	redimension_coin(void)
{
	int	slot;

	start_one(&slot);
	hs_drag(&g_s, (t_point){219, 169}, (t_point){249, 199});
	h_eq_i64("fenetre agrandie", g_s.t.w[slot].rect.w, 230);
	ws_frame(&g_s);
	h_eq_i64("incremental = complet apres le relachement",
		pixels_differents(), 0);
	hs_stop(&g_s);
}

static void	section_reattachee(void)
{
	int	slot;

	start_one(&slot);
	ws_section_release(&g_s, slot);
	ws_mark_win(&g_s, slot);
	ws_frame(&g_s);
	hs_drag(&g_s, (t_point){100, 30}, (t_point){130, 60});
	h_true(g_s.t.w[slot].content.px != NULL, "section reattachee");
	ws_frame(&g_s);
	h_eq_i64("incremental = complet apres reattachement",
		pixels_differents(), 0);
	hs_stop(&g_s);
}

int	main(void)
{
	h_begin("a18/srv_section_mark");
	h_run("transition d'etat : redimensionnement par le coin",
		redimension_coin);
	h_run("transition d'etat : section perdue puis reattachee",
		section_reattachee);
	return (h_end());
}
