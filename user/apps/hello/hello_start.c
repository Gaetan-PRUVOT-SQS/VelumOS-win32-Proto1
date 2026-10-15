#include "../common/platform.h"
#include "hello.h"

int	main(void)
{
	t_hello	app;

	if (hello_open(&app) < 0)
	{
		os_log("hello: connexion au serveur de fenêtres impossible");
		return (1);
	}
	os_log("hello: fenêtre prête");
	hello_run(&app);
	uiwin_close(&app.ui);
	return (0);
}
