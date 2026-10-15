#include "ws_srv.h"

void	ws_run(t_wsrv *s)
{
	uint32_t	errors;

	errors = 0;
	while (!s->quit)
	{
		if (ws_step(s) < 0)
			errors++;
		else
			errors = 0;
		if (errors >= 100)
		{
			ws_sys_log("winsrv: attente en echec, arret");
			s->quit = true;
		}
	}
}

int	main(void)
{
	static t_wsrv	srv;

	if (ws_open(&srv) < 0)
	{
		ws_sys_log("winsrv: ecran ou port indisponible");
		return (1);
	}
	ws_run(&srv);
	ws_close_all(&srv);
	return (0);
}
