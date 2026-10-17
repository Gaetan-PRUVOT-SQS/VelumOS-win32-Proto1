#include "velum/abi/abi_syscall.h"
#include "velum/libk.h"
#include "../common/platform.h"
#include "../common/tbuf.h"
#include "../common/typo.h"
#include "../common/utf8.h"
#include "runcmd.h"
#include "shell.h"

void	sh_logoff(t_shell *sh)
{
	sh->quit = 1;
}

void	sh_power(t_shell *sh, uint32_t op)
{
	if (os_power(op) < 0)
		sh_note(sh, "Éteindre l'ordinateur",
			"Impossible d'arrêter l'ordinateur" NBSP ": droit refusé.");
}

static int	run_apk(t_shell *sh, const char *text, size_t len)
{
	char	apk[RUN_APK_MAX];

	if (run_apk_path(text, len, apk, sizeof(apk)) < 0)
		return (0);
	return (sh_launch_arg(sh, SH_APKRUN_PATH, apk) == 0);
}

void	sh_run_command(t_shell *sh, const char *text)
{
	char	path[RUN_NAME_MAX + 16];
	char	shown[SH_LABEL_MAX];
	char	msg[SH_NOTE_MAX];
	t_tbuf	b;
	size_t	len;

	len = strnlen(text, CTL_TEXT_MAX);
	if (run_apk(sh, text, len))
		return ;
	if (run_resolve(text, len, path, sizeof(path)) == 0
		&& sh_launch(sh, path) == 0)
		return ;
	utf8_clean_copy(shown, sizeof(shown), text, len);
	tb_init(&b, msg, sizeof(msg));
	tb_str(&b, "Impossible de trouver «" NBSP);
	tb_str(&b, shown);
	tb_str(&b, NBSP "».\nVérifiez le nom, puis réessayez.");
	sh_note(sh, "Exécuter", msg);
}

void	sh_desk_action(t_shell *sh, uint32_t action)
{
	if (action == DA_HELLO)
		sh_launch(sh, SH_HELLO_PATH);
	else if (action == DA_COMPUTER)
		sh_note(sh, "Ordinateur",
			"Cette fonction n'est pas encore disponible dans VelumOS.");
	else if (action == DA_RECYCLE)
		sh_note(sh, "Corbeille", "La corbeille est vide.");
}
