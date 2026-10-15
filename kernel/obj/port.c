#include "obj_int.h"
#include "velum/libk.h"

t_ports	g_ports;

void	port_init(void)
{
	memset(&g_ports, 0, sizeof(t_ports));
	spin_init(&g_ports.lock, "ports");
}

static bool	name_char_ok(char c)
{
	if (c >= 'a' && c <= 'z')
		return (true);
	if (c >= '0' && c <= '9')
		return (true);
	return (c == '_' || c == '.' || c == '-');
}

bool	port_name_ok(const char *name, uint64_t len)
{
	uint64_t	i;

	if (!name || len == 0 || len > IPC_NAME_MAX)
		return (false);
	i = 0;
	while (i < len)
	{
		if (!name_char_ok(name[i]))
			return (false);
		i++;
	}
	return (true);
}

t_listener	*port_find(const char *name, uint32_t len)
{
	t_listener	*l;
	uint32_t	i;

	l = g_ports.head;
	while (l)
	{
		i = 0;
		while (l->namelen == len && i < len && l->name[i] == name[i])
			i++;
		if (l->namelen == len && i == len
			&& __atomic_load_n(&l->obj->refs, __ATOMIC_ACQUIRE) != 0)
			return (l);
		l = l->next;
	}
	return (NULL);
}

int	port_listen(const char *name, uint32_t len, t_object **out)
{
	t_listener	*l;
	uint64_t	fl;
	int			rc;

	if (!port_name_ok(name, len) || !out)
		return (E_INVAL);
	rc = listener_new(name, len, out);
	if (rc < 0)
		return (rc);
	l = (*out)->impl;
	fl = spin_lock_irqsave(&g_ports.lock);
	rc = E_EXIST;
	if (!port_find(name, len))
	{
		l->next = g_ports.head;
		g_ports.head = l;
		l->registered = 1;
		rc = 0;
	}
	spin_unlock_irqrestore(&g_ports.lock, fl);
	if (rc == 0)
		return (0);
	obj_unref(*out);
	*out = NULL;
	return (rc);
}
