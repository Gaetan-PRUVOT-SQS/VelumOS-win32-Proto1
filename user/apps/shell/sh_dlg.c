#include "velum/abi/abi_syscall.h"
#include "velum/libk.h"
#include "../common/layout.h"
#include "sh_desk.h"
#include "shell.h"

static t_rect	dlg_rect(const t_shell *sh, int kind)
{
	int32_t	w;
	int32_t	h;
	int32_t	sw;
	int32_t	area_h;

	w = SH_DLGW_RUN;
	h = SH_DLGH_RUN;
	if (kind == SH_DLG_POWER)
	{
		w = SH_DLGW_POWER;
		h = SH_DLGH_POWER;
	}
	if (kind == SH_DLG_NOTE)
	{
		w = SH_DLGW_NOTE;
		h = SH_DLGH_NOTE;
	}
	w += 2 * sh->lm.frame_w;
	h += sh->lm.caption_h + sh->lm.frame_bottom;
	sw = (int32_t)sh->info.screen_w;
	area_h = (int32_t)sh->info.screen_h - sh->lm.taskbar_h;
	return (lay_rect((sw - w) / 2, (area_h - h) / 2, w, h));
}

void	sh_dlg_close(t_shell *sh)
{
	if (!sh->dlg.ready)
		return ;
	uiwin_close(&sh->dlg);
	sh->dlg_kind = 0;
	sh->dlg_act = SHA_NONE;
}

void	sh_dlg_open(t_shell *sh, int kind, const char *title)
{
	t_wmcreate	rq;

	if (sh->dlg.ready)
	{
		wmc_activate(sh->dlg.win.id);
		return ;
	}
	ui_request(&rq, dlg_rect(sh, kind), WS_CAPTION | WS_SYSMENU, title);
	if (uiwin_open(&sh->dlg, &rq, sh_dlg_command) < 0)
		return ;
	sh->dlg.root.user = sh;
	sh->dlg_kind = kind;
	if (sh_dlg_build(sh, kind) < 0)
	{
		sh_dlg_close(sh);
		return ;
	}
	uiwin_flush(&sh->dlg);
}

void	sh_note(t_shell *sh, const char *title, const char *text)
{
	if (sh->dlg.ready)
		sh_dlg_close(sh);
	strlcpy(sh->note, text, sizeof(sh->note));
	sh_dlg_open(sh, SH_DLG_NOTE, title);
}

void	sh_dlg_finish(t_shell *sh)
{
	char	arg[CTL_TEXT_MAX];
	int		act;

	act = sh->dlg_act;
	if (act == SHA_NONE)
		return ;
	strlcpy(arg, sh->arg, sizeof(arg));
	memset(sh->arg, 0, sizeof(sh->arg));
	sh_dlg_close(sh);
	if (act == SHA_RUN)
		sh_run_command(sh, arg);
	else if (act == SHA_HALT)
		sh_power(sh, POWER_OFF);
	else if (act == SHA_REBOOT)
		sh_power(sh, POWER_REBOOT);
}
