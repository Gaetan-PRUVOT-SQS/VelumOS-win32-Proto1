#include "sys_int.h"
#include "velum/vobj.h"

int64_t	v_port_connect(const char *name, uint32_t flags)
{
	int64_t	len;

	len = sys_cstrlen(name, IPC_NAME_MAX);
	if (len < 0)
		return (len);
	return (sys3(SYS_PORT_CONNECT, sys_ptr(name), (uint64_t)len, flags));
}

int64_t	v_port_accept(t_handle listener, uint64_t timeout_ns)
{
	return (sys2(SYS_PORT_ACCEPT, listener, timeout_ns));
}

int	v_chan_send(t_handle chan, const t_chansend *msg)
{
	return ((int)sys2(SYS_CHAN_SEND, chan, sys_ptr(msg)));
}

int	v_chan_recv(t_handle chan, t_chanrecv *msg)
{
	return ((int)sys2(SYS_CHAN_RECV, chan, sys_ptr(msg)));
}

int64_t	v_timer_create(int manual)
{
	return (sys1(SYS_TIMER_CREATE, manual != 0));
}
