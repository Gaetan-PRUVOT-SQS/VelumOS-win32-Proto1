#include <stdlib.h>
#include "velum/err.h"
#include "apkrun.h"

int	apkrun_check(t_apkrun *app, int r)
{
	if (r == 0)
		return (0);
	if (r == DVM_THROWN)
		return (apkrun_refuse(app, app->d->error));
	if (r == E_TIMEOUT)
		return (apkrun_refuse(app, T_BUDGET));
	if (r == E_NOMEM)
		return (apkrun_refuse(app, T_MEM));
	if (r == E_NOTSUP)
		return (apkrun_refuse(app, T_NOTSUP));
	return (apkrun_refuse(app, T_VM));
}

int	apkrun_start(t_apkrun *app)
{
	app->vm->spent = 0;
	if (apkrun_check(app, droid_start(app->d, app->man.activity_desc)) < 0)
		return (-1);
	app->started = 1;
	app->redraw = 1;
	apkrun_log_step(app, "prêt", "");
	return (0);
}

void	apkrun_click(t_apkrun *app, uint32_t view)
{
	const t_droidview	*v;

	v = droid_view(app->d, view);
	if (!v || v->kind != DV_BUTTON)
		return ;
	app->vm->spent = 0;
	apkrun_check(app, droid_click(app->d, view));
}

void	apkrun_close(t_apkrun *app)
{
	if (app->started && !app->dead)
	{
		app->vm->spent = 0;
		(void)droid_stop(app->d);
	}
	if (app->vm)
		dvm_destroy(app->vm);
	free(app->d);
	free(app->dex);
	free(app->file);
	app->vm = NULL;
	app->d = NULL;
	app->dex = NULL;
	app->file = NULL;
	app->started = 0;
	if (app->open)
		uiwin_close(&app->ui);
	app->open = 0;
}
