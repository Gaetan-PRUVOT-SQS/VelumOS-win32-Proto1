#include "velum/libk.h"
#include "shell.h"

static void	submit_run(t_shell *sh)
{
	const t_ctl	*edit;

	edit = ctl_find(&sh->dlg.root, SH_ID_EDIT);
	sh->dlg_act = SHA_CLOSE;
	if (!edit || edit->text[0] == '\0')
		return ;
	strlcpy(sh->arg, edit->text, sizeof(sh->arg));
	sh->dlg_act = SHA_RUN;
}

void	sh_dlg_command(void *c, uint32_t code, void *user)
{
	const t_ctl	*ctl;
	t_shell		*sh;

	ctl = c;
	sh = user;
	if (!sh || (code != CN_CLICKED && code != CN_ENTER))
		return ;
	if (ctl->id == CTL_ID_CANCEL)
		sh->dlg_act = SHA_CLOSE;
	if ((ctl->id == CTL_ID_OK || ctl->id == SH_ID_EDIT)
		&& sh->dlg_kind == SH_DLG_RUN)
		submit_run(sh);
	if (ctl->id == SH_ID_HALT)
		sh->dlg_act = SHA_HALT;
	if (ctl->id == SH_ID_REBOOT)
		sh->dlg_act = SHA_REBOOT;
}
