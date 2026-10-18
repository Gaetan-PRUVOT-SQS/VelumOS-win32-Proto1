#include "velum/libk.h"
#include "../common/platform.h"
#include "notif_app.h"

int	main(int argc, char **argv)
{
	t_notifapp	app;
	int			open_now;

	open_now = argc > 1 && strcmp(argv[1], NA_ARG_OPEN) == 0;
	if (notifapp_open(&app, open_now) < 0)
	{
		os_log("notif: demarrage impossible");
		notifapp_close(&app);
		return (1);
	}
	os_log("notif: service pret");
	notifapp_run(&app);
	notifapp_close(&app);
	return (0);
}
