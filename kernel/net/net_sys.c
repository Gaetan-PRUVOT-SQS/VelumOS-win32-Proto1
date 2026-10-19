#include "velum/err.h"
#include "velum/heap.h"
#include "velum/ksyscall.h"
#include "velum/libk.h"
#include "velum/proc.h"
#include "velum/vmm.h"
#include "net_int.h"

#define NET_PING_MAX_NS 10000000000ull

typedef struct s_sockref
{
	uint32_t	id;
	uint32_t	owner;
}	t_sockref;

static bool	pid_alive(uint32_t pid)
{
	return (proc_find(pid) != NULL);
}

static t_process	*caller(void)
{
	t_process	*p;

	p = proc_current();
	if (!p || !(p->flags & PF_NET))
		return (NULL);
	return (p);
}

static int64_t	sys_net_info(const t_sysargs *args)
{
	t_netinfo	info;

	if (!caller())
		return (E_PERM);
	net_info(&info);
	return (copy_to_user(args->a[0], &info, sizeof(info)));
}

static int64_t	sys_net_socket(const t_sysargs *args)
{
	t_process	*p;
	int			rc;

	(void)args;
	p = caller();
	if (!p)
		return (E_PERM);
	mutex_lock(&g_net.lock);
	rc = sock_alloc_l(p->pid, pid_alive);
	mutex_unlock(&g_net.lock);
	return (rc);
}

static int64_t	sys_net_bind(const t_sysargs *args)
{
	t_process	*p;
	t_sock		*s;
	int			rc;

	p = caller();
	if (!p)
		return (E_PERM);
	if (args->a[1] > 0xffff)
		return (E_INVAL);
	mutex_lock(&g_net.lock);
	s = sock_get_l((uint32_t)args->a[0], p->pid);
	rc = E_BADF;
	if (s)
		rc = sock_bind_l(s, (uint16_t)args->a[1], (p->flags & PF_ADMIN) != 0);
	mutex_unlock(&g_net.lock);
	return (rc);
}

static int	send_locked(t_process *p, uint32_t id, const t_netaddr *to,
		const uint8_t *data, size_t len)
{
	t_sock	*s;
	int		rc;

	mutex_lock(&g_net.lock);
	s = sock_get_l(id, p->pid);
	rc = E_BADF;
	if (s && !s->bound)
		rc = sock_bind_l(s, 0, false);
	else if (s)
		rc = 0;
	if (rc >= 0)
		rc = net_udp_send_l(s->port, to->ip, to->port, data, len);
	mutex_unlock(&g_net.lock);
	return (rc);
}

static int64_t	sys_net_sendto(const t_sysargs *args)
{
	t_process	*p;
	t_netaddr	to;
	uint8_t		*data;
	int			rc;

	p = caller();
	if (!p)
		return (E_PERM);
	if (args->a[2] > NET_UDP_MAX)
		return (E_INVAL);
	if (copy_from_user(&to, args->a[3], sizeof(to)) < 0)
		return (E_FAULT);
	if (to.port == 0 || to.ip == 0)
		return (E_INVAL);
	data = kmalloc(args->a[2] ? args->a[2] : 1);
	if (!data)
		return (E_NOMEM);
	rc = copy_from_user(data, args->a[1], args->a[2]);
	if (rc == 0)
		rc = send_locked(p, (uint32_t)args->a[0], &to, data, args->a[2]);
	else
		rc = E_FAULT;
	kfree(data);
	return (rc < 0 ? rc : (int64_t)args->a[2]);
}

static bool	sockref_ready(void *arg)
{
	t_sockref	*r;
	t_sock		*s;

	r = arg;
	s = sock_get_l(r->id, r->owner);
	return (!s || s->count > 0);
}

static int	take_dgram(const t_sockref *r, uint64_t timeout_ns, t_dgram *d)
{
	t_sock	*s;
	int		rc;

	mutex_lock(&g_net.lock);
	s = sock_get_l(r->id, r->owner);
	rc = s ? sock_take_l(s, d) : E_BADF;
	mutex_unlock(&g_net.lock);
	if (rc != E_AGAIN || timeout_ns == 0)
		return (rc);
	if (net_wait(sockref_ready, (void *)r, timeout_ns) < 0)
		return (E_TIMEOUT);
	mutex_lock(&g_net.lock);
	s = sock_get_l(r->id, r->owner);
	rc = s ? sock_take_l(s, d) : E_BADF;
	mutex_unlock(&g_net.lock);
	return (rc);
}

static int64_t	deliver_dgram(const t_sysargs *args, const t_dgram *d)
{
	t_netaddr	from;
	size_t		n;

	n = d->len < args->a[2] ? d->len : args->a[2];
	if (copy_to_user(args->a[1], d->data, n) < 0)
		return (E_FAULT);
	if (args->a[3])
	{
		from.ip = d->ip;
		from.port = d->port;
		from.reserve = 0;
		if (copy_to_user(args->a[3], &from, sizeof(from)) < 0)
			return (E_FAULT);
	}
	return ((int64_t)n);
}

static int64_t	sys_net_recvfrom(const t_sysargs *args)
{
	t_process	*p;
	t_sockref	r;
	t_dgram		*d;
	int64_t		rc;

	p = caller();
	if (!p)
		return (E_PERM);
	if (args->a[2] > NET_UDP_MAX)
		return (E_INVAL);
	d = kmalloc(sizeof(*d));
	if (!d)
		return (E_NOMEM);
	r.id = (uint32_t)args->a[0];
	r.owner = p->pid;
	rc = take_dgram(&r, args->a[4], d);
	if (rc == 0)
		rc = deliver_dgram(args, d);
	kfree(d);
	return (rc);
}

static int64_t	sys_net_close(const t_sysargs *args)
{
	t_process	*p;
	t_sock		*s;
	int			rc;

	p = caller();
	if (!p)
		return (E_PERM);
	mutex_lock(&g_net.lock);
	s = sock_get_l((uint32_t)args->a[0], p->pid);
	rc = E_BADF;
	if (s)
	{
		sock_free_l(s);
		rc = 0;
	}
	mutex_unlock(&g_net.lock);
	return (rc);
}

static int64_t	sys_net_ping(const t_sysargs *args)
{
	uint64_t	rtt;
	uint64_t	timeout;
	int			rc;

	if (!caller())
		return (E_PERM);
	timeout = args->a[1];
	if (timeout == 0 || timeout > NET_PING_MAX_NS)
		timeout = NET_PING_MAX_NS;
	rtt = 0;
	rc = net_ping((uint32_t)args->a[0], timeout, &rtt);
	if (rc < 0)
		return (rc);
	return ((int64_t)(rtt / 1000));
}

int	net_sys_register(void)
{
	int	fails;

	fails = 0;
	fails += syscall_register(SYS_NET_INFO, sys_net_info, "net_info") < 0;
	fails += syscall_register(SYS_NET_SOCKET, sys_net_socket, "net_socket") < 0;
	fails += syscall_register(SYS_NET_BIND, sys_net_bind, "net_bind") < 0;
	fails += syscall_register(SYS_NET_SENDTO, sys_net_sendto, "net_sendto") < 0;
	fails += syscall_register(SYS_NET_RECVFROM, sys_net_recvfrom,
			"net_recvfrom") < 0;
	fails += syscall_register(SYS_NET_CLOSE, sys_net_close, "net_close") < 0;
	fails += syscall_register(SYS_NET_PING, sys_net_ping, "net_ping") < 0;
	return (fails ? E_IO : 0);
}
