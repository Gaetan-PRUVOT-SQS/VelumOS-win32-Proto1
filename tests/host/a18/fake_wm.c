#include "velum/abi/abi_syscall.h"
#include "velum/err.h"
#include "fake_kern.h"
#include "wmc_int.h"

int64_t	wmsys_connect(const char *name)
{
	if (name == NULL)
		return (E_INVAL);
	return (fk_connect());
}

int	wmsys_send(t_handle ch, const void *msg, uint32_t len)
{
	return (fk_send(ch, msg, len, 0));
}

static int	pumped(t_handle ch, t_wmcmsg *m)
{
	int	r;
	int	k;

	r = fk_recv(ch, m->buf, sizeof(m->buf), &m->handle);
	k = 0;
	while (r == E_TIMEOUT && g_fk.pump != NULL && !g_fk.pumping && k < 8)
	{
		g_fk.pumping = 1;
		g_fk.pump(g_fk.pump_arg);
		g_fk.pumping = 0;
		r = fk_recv(ch, m->buf, sizeof(m->buf), &m->handle);
		k++;
	}
	return (r);
}

int	wmsys_recv(t_handle ch, t_wmcmsg *m, uint64_t timeout_ns)
{
	int	r;

	m->handle = 0;
	r = pumped(ch, m);
	if (r == E_TIMEOUT && timeout_ns != TIMEOUT_INF)
		g_fk.now += timeout_ns;
	if (r >= 0)
		m->len = (uint32_t)r;
	return (r);
}

void	*wmsys_map(t_handle section, uint64_t len)
{
	return (fk_map(section, len));
}
