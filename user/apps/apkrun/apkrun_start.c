#include "../common/platform.h"
#include "apkrun.h"

int	main(int argc, char **argv)
{
	static t_apkrun	app;
	int				loaded;

	if (argc < 2)
	{
		os_log("apkrun: refusé : chemin de l'APK manquant");
		return (1);
	}
	loaded = apkrun_load(&app, argv[1]);
	if (apkrun_open(&app) < 0)
	{
		os_log("apkrun: connexion au serveur de fenêtres impossible");
		apkrun_close(&app);
		return (1);
	}
	if (loaded == 0)
		apkrun_start(&app);
	apkrun_run(&app);
	loaded = app.dead;
	apkrun_close(&app);
	return (loaded != 0);
}
