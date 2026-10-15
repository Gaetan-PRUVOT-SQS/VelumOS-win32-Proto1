#include "velum/abi/abi_syscall.h"
#include "help.h"
#include "wmc_int.h"

void	hs_pump(void *arg)
{
	ws_step(arg);
}

int	hs_start(t_wsrv *s, uint32_t w, uint32_t h)
{
	int	r;

	fk_reset();
	fk_set_display(w, h);
	r = ws_open(s);
	g_fk.pump = hs_pump;
	g_fk.pump_arg = s;
	return (r);
}

void	hs_stop(t_wsrv *s)
{
	wmc_disconnect();
	ws_close_all(s);
	fk_reset();
}

void	hs_settle(t_wsrv *s, t_handle c)
{
	int	obj;
	int	k;

	obj = fk_obj_of(c, FK_CHAN);
	k = 0;
	while (obj >= 0 && g_fk.o[obj].peer >= 0
		&& g_fk.o[g_fk.o[obj].peer].n > 0 && k < 200)
	{
		ws_step(s);
		k++;
	}
	ws_step(s);
}

void	hs_input(t_wsrv *s)
{
	int	k;

	k = 0;
	while (g_fk.in_n > 0 && k < 64)
	{
		ws_step(s);
		k++;
	}
	g_fk.now += 20000000ull;
	ws_step(s);
}
