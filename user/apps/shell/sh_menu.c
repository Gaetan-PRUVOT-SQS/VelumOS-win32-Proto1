#include "shell.h"

static void	sync(t_shell *sh)
{
	if (sh->sm.open)
	{
		sh_menu_paint(sh);
		return ;
	}
	if (sh->menu_shown)
	{
		wmc_capture(&sh->menu, 0);
		wmc_set_state(&sh->menu, WSTATE_HIDDEN);
		sh->menu_shown = 0;
	}
	sh_bar_paint(sh);
}

void	sh_menu_open(t_shell *sh)
{
	if (sh->sm.open)
		return ;
	sm_open(&sh->sm);
	sh_menu_paint(sh);
	wmc_set_state(&sh->menu, WSTATE_NORMAL);
	wmc_capture(&sh->menu, 1);
	sh->menu_shown = 1;
	sh_bar_paint(sh);
}

void	sh_menu_close(t_shell *sh)
{
	sm_close(&sh->sm);
	sync(sh);
}

void	sh_menu_toggle(t_shell *sh)
{
	if (sh->sm.open)
		sh_menu_close(sh);
	else
		sh_menu_open(sh);
}

void	sh_menu_run(t_shell *sh, int cmd)
{
	sync(sh);
	if (sm_prog_of((uint32_t)cmd) >= 0)
		sh_launch_program(sh, (uint32_t)cmd);
	else if (cmd == SMC_RUN)
		sh_dlg_open(sh, SH_DLG_RUN, "Exécuter");
	else if (cmd == SMC_LOGOFF)
		sh_logoff(sh);
	else if (cmd == SMC_SHUTDOWN)
		sh_dlg_open(sh, SH_DLG_POWER, "Éteindre l'ordinateur");
}
