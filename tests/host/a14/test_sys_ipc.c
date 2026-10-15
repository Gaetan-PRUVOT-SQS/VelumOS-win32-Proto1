#include "fake_check.h"
#include "fake_sys.h"
#include "harness.h"
#include "velum/velum.h"

static int64_t	g_ret;

static void	ipc_ports(void)
{
	const char	*name = "wm";

	fake_reset();
	g_fsys.ret = g_ret;
	h_eq_i64("port_listen", v_port_listen(name, 0x31), g_ret);
	sys_is(SYS_PORT_LISTEN, sys_u(name), 2, 0x31);
	sys_is_hi(0, 0, 0);
	h_eq_i64("port_connect", v_port_connect(name, 0x32), g_ret);
	sys_is(SYS_PORT_CONNECT, sys_u(name), 2, 0x32);
	h_eq_i64("port_accept", v_port_accept(0x33, 0x4444), g_ret);
	sys_is(SYS_PORT_ACCEPT, 0x33, 0x4444, 0);
	h_eq_i64("accept sans fin", v_port_accept(0x34, TIMEOUT_INF), g_ret);
	sys_is(SYS_PORT_ACCEPT, 0x34, TIMEOUT_INF, 0);
}

static void	ipc_channels(void)
{
	t_chansend	snd;
	t_chanrecv	rcv;

	fake_reset();
	g_fsys.ret = g_ret;
	h_eq_i64("chan_send", v_chan_send(0x41, &snd), g_ret);
	sys_is(SYS_CHAN_SEND, 0x41, sys_u(&snd), 0);
	sys_is_hi(0, 0, 0);
	h_eq_i64("chan_recv", v_chan_recv(0x42, &rcv), g_ret);
	sys_is(SYS_CHAN_RECV, 0x42, sys_u(&rcv), 0);
	h_eq_i64("chan_send pointeur nul transmis", v_chan_send(1, NULL), g_ret);
	sys_is(SYS_CHAN_SEND, 1, 0, 0);
}

static void	ipc_timers(void)
{
	fake_reset();
	g_fsys.ret = g_ret;
	h_eq_i64("timer_create", v_timer_create(1), g_ret);
	sys_is(SYS_TIMER_CREATE, 1, 0, 0);
	h_eq_i64("timer_set", v_timer_set(0x51, 111, 222), g_ret);
	sys_is(SYS_TIMER_SET, 0x51, 111, 222);
	sys_is_hi(0, 0, 0);
	h_eq_i64("timer_set annulation", v_timer_set(0x52, 0, 0), g_ret);
	sys_is(SYS_TIMER_SET, 0x52, 0, 0);
	h_eq_i64("timer_create booleen 5", v_timer_create(5), g_ret);
	sys_is(SYS_TIMER_CREATE, 1, 0, 0);
	h_eq_i64("timer_create 0", v_timer_create(0), g_ret);
	sys_is(SYS_TIMER_CREATE, 0, 0, 0);
}

int	main(void)
{
	h_begin("a14/sys_ipc");
	g_ret = 9;
	h_run("vobj/exigence : ports, retour positif", ipc_ports);
	h_run("vobj/exigence : canaux, retour positif", ipc_channels);
	h_run("vobj/exigence : minuteries et booleens, retour positif",
		ipc_timers);
	g_ret = E_AGAIN;
	h_run("vobj/exigence : ports, retour negatif", ipc_ports);
	h_run("vobj/exigence : canaux, retour negatif", ipc_channels);
	h_run("vobj/exigence : minuteries et booleens, retour negatif",
		ipc_timers);
	return (h_end());
}
