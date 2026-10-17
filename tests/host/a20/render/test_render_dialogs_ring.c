#include "harness.h"
#include "velum/libk.h"
#include "render.h"

static const char	*g_argv[] = {"shell", "Utilisateur", NULL};

static int	rect_is(const t_ctl *c, t_rect e)
{
	return (c != NULL && c->rect.x == e.x && c->rect.y == e.y
		&& c->rect.w == e.w && c->rect.h == e.h);
}

static void	dialogues_boutons_rectangle_exterieur(void)
{
	t_shell		sh;
	t_ctlroot	*r;

	render_reset(A20_FIXTURE);
	sh_boot(&sh, 2, (char **)g_argv);
	r = &sh.dlg.root;
	drv_menu_item(&sh, SM_IDX_RUN);
	h_true(rect_is(ctl_find(r, CTL_ID_OK), rect_make(188, 78, 88, 34)),
		"Executer: OK 80x26 en (192,82) + anneau");
	h_true(rect_is(ctl_find(r, CTL_ID_CANCEL), rect_make(272, 78, 88, 34)),
		"Executer: Annuler 80x26 en (276,82) + anneau");
	sh_dlg_close(&sh);
	drv_menu_item(&sh, SM_IDX_SHUTDOWN);
	h_true(rect_is(ctl_find(r, SH_ID_HALT), rect_make(12, 58, 108, 34)),
		"Extinction: Arreter 100x26 en (16,62) + anneau");
	h_true(rect_is(ctl_find(r, SH_ID_REBOOT), rect_make(120, 58, 108, 34)),
		"Extinction: Redemarrer 100x26 en (124,62) + anneau");
	h_true(rect_is(ctl_find(r, CTL_ID_CANCEL), rect_make(228, 58, 108, 34)),
		"Extinction: Annuler 100x26 en (232,62) + anneau");
	sh_dlg_close(&sh);
	sh_note(&sh, "Note", "Message");
	h_true(rect_is(ctl_find(r, CTL_ID_CANCEL), rect_make(232, 58, 88, 34)),
		"Note: OK 80x26 en (236,62) + anneau");
	sh_shutdown(&sh);
}

int	main(void)
{
	h_begin("a20/render-dialogs-ring");
	h_run("rendu dialogues: boutons exterieurs",
		dialogues_boutons_rectangle_exterieur);
	return (h_end());
}
