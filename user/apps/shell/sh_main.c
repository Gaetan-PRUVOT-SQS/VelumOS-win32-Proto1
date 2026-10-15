#include <stdlib.h>
#include "velum/err.h"
#include "velum/libk.h"
#include "../common/platform.h"
#include "../common/utf8.h"
#include "shell.h"

static void	set_user(t_shell *sh, int argc, char **argv)
{
	strlcpy(sh->user, SH_DEFAULT_USER, sizeof(sh->user));
	if (argc > 1 && argv[1] && argv[1][0])
		utf8_clean_copy(sh->user, sizeof(sh->user), argv[1],
			strnlen(argv[1], SH_USER_MAX));
}

int	sh_boot(t_shell *sh, int argc, char **argv)
{
	int	r;

	memset(sh, 0, sizeof(*sh));
	sh->selected = DESK_NONE;
	sh->hot_task = -1;
	sm_init(&sh->sm);
	sh_load_programs(sh);
	dc_reset(&sh->last_click);
	set_user(sh, argc, argv);
	luna_metrics(&sh->lm);
	r = ui_connect(&sh->info);
	if (r < 0)
		return (r);
	if (os_timer_open(&sh->timer) < 0)
		return (E_NOMEM);
	r = sh_make_windows(sh);
	if (r < 0)
		return (r);
	sh_wall_make(sh);
	sh_clock_tick(sh);
	sh_desk_paint(sh);
	wmc_subscribe(1);
	return (0);
}

void	sh_shutdown(t_shell *sh)
{
	sh_dlg_close(sh);
	sh_kill_children(sh);
	wmc_destroy(&sh->menu);
	wmc_destroy(&sh->bar);
	wmc_destroy(&sh->desk);
	free(sh->wall_px);
	sh_glass_free(sh);
	wmc_disconnect();
}
