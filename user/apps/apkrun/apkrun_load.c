#include <stdlib.h>
#include "velum/err.h"
#include "velum/libk.h"
#include "../common/apkglue.h"
#include "apkrun.h"

static const char	*why(int reason)
{
	if (reason <= APKR_OK || reason >= APKR_COUNT)
		return (T_APK);
	return (apk_reason(reason));
}

static int	open_apk(t_apkrun *app, int64_t len)
{
	t_apksig	sig;
	int			r;

	memset(&sig, 0, sizeof(sig));
	r = apk_open(&app->apk, (t_span){app->file, (size_t)len});
	if (r < 0)
		return (apkrun_refuse(app, why(app->apk.reason)));
	r = apk_verify(&app->apk, &sig);
	if (r < 0 && sig.reason == APKR_OK)
		sig.reason = APKR_SIGNATURE;
	if (r < 0)
		return (apkrun_refuse(app, why(sig.reason)));
	r = apk_manifest(&app->apk, &app->man);
	if (r < 0)
		return (apkrun_refuse(app, why(app->man.reason)));
	app->known = 1;
	apkrun_log_step(app, "signature ok", "");
	return (0);
}

static int	make_vm(t_apkrun *app)
{
	t_dlimits	lim;

	lim = (t_dlimits){DVM_HEAP_DEFAULT, DVM_STACK_WORDS_DEFAULT,
		DVM_DEPTH_DEFAULT, DVM_CLASSES_DEFAULT, APKRUN_BUDGET};
	app->d = malloc(sizeof(*app->d));
	if (!app->d)
		return (apkrun_refuse(app, T_MEM));
	memset(app->d, 0, sizeof(*app->d));
	if (dvm_create(&app->vm, &lim) < 0)
	{
		app->vm = NULL;
		return (apkrun_refuse(app, T_MEM));
	}
	if (droid_install(app->d, app->vm) < 0)
		return (apkrun_refuse(app, T_MEM));
	app->d->log = apkrun_applog;
	app->d->clock = apkrun_clock;
	app->d->user = app;
	return (0);
}

static int	load_dex(t_apkrun *app)
{
	int64_t	n;
	int		r;

	n = apk_read(&app->apk, "classes.dex", &app->dex);
	if (n < 0)
	{
		app->dex = NULL;
		return (apkrun_refuse(app, T_DEX));
	}
	r = dvm_load_dex(app->vm, (t_span){app->dex, (size_t)n});
	if (r == E_NOTSUP)
		return (apkrun_refuse(app, T_NOTSUP));
	if (r == E_NOMEM)
		return (apkrun_refuse(app, T_MEM));
	if (r != 0)
		return (apkrun_refuse(app, T_DEX));
	return (0);
}

int	apkrun_load(t_apkrun *app, const char *path)
{
	int64_t	n;

	app->path = path;
	n = apkglue_read_file(path, &app->file);
	if (n < 0)
		return (apkrun_refuse(app, T_FILE));
	if (open_apk(app, n) < 0 || make_vm(app) < 0 || load_dex(app) < 0)
		return (-1);
	return (0);
}
