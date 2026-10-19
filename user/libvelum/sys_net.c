#include "sys_int.h"
#include "velum/vnet.h"

int	v_net_info(t_netinfo *out)
{
	return ((int)sys1(SYS_NET_INFO, sys_ptr(out)));
}

int	v_net_socket(void)
{
	return ((int)sys0(SYS_NET_SOCKET));
}

int	v_net_bind(int sock, uint16_t port)
{
	return ((int)sys2(SYS_NET_BIND, (uint64_t)sock, port));
}

int	v_net_sendto(int sock, const void *buf, uint32_t len, const t_netaddr *to)
{
	uint64_t	args[V_SYS_ARGS];

	args[0] = (uint64_t)sock;
	args[1] = sys_ptr(buf);
	args[2] = len;
	args[3] = sys_ptr(to);
	args[4] = 0;
	args[5] = 0;
	return ((int)v_syscall6(SYS_NET_SENDTO, args));
}

int	v_net_recvfrom(int sock, void *buf, uint32_t len, t_netaddr *from,
		uint64_t timeout_ns)
{
	uint64_t	args[V_SYS_ARGS];

	args[0] = (uint64_t)sock;
	args[1] = sys_ptr(buf);
	args[2] = len;
	args[3] = sys_ptr(from);
	args[4] = timeout_ns;
	args[5] = 0;
	return ((int)v_syscall6(SYS_NET_RECVFROM, args));
}

int	v_net_close(int sock)
{
	return ((int)sys1(SYS_NET_CLOSE, (uint64_t)sock));
}

int	v_net_ping(uint32_t ip, uint64_t timeout_ns)
{
	return ((int)sys2(SYS_NET_PING, ip, timeout_ns));
}
