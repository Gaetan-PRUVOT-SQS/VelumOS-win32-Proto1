#include "../common/platform.h"
#include "fichiers.h"

static t_fichiers	g_app;

int	main(void)
{
	if (ff_app_open(&g_app) < 0)
	{
		os_log("fichiers: connexion au serveur de fenêtres impossible");
		return (1);
	}
	os_log("fichiers: fenêtre prête");
	ff_app_run(&g_app);
	uiwin_close(&g_app.ui);
	return (0);
}
