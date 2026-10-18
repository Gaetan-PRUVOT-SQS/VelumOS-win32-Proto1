#include "../common/platform.h"
#include "reglages.h"

int	main(void)
{
	t_reglages	app;

	if (reg_open(&app) < 0)
	{
		os_log("reglages: connexion au serveur de fenêtres impossible");
		return (1);
	}
	os_log("reglages: fenêtre prête");
	reg_run(&app);
	uiwin_close(&app.ui);
	return (0);
}
