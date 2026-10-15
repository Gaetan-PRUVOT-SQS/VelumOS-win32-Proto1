#include "help.h"

void	hr_activate(t_wsrv *s, t_handle c, uint32_t id)
{
	t_wmarg	m;

	hr_arg(&m, WMC_ACTIVATE, id, 0);
	m.h.size = sizeof(t_wmhdr);
	hr_send(s, c, &m);
}

void	hr_destroy(t_wsrv *s, t_handle c, uint32_t id)
{
	t_wmarg	m;

	hr_arg(&m, WMC_DESTROY, id, 0);
	m.h.size = sizeof(t_wmhdr);
	hr_send(s, c, &m);
}

void	hr_value(t_wsrv *s, t_handle c, uint32_t type, uint32_t iv[2])
{
	t_wmarg	m;

	hr_arg(&m, type, iv[0], iv[1]);
	hr_send(s, c, &m);
}

int32_t	hr_error(t_handle c)
{
	t_wmhdr	e;

	e.status = 0;
	if (hr_drain(c, WMS_ERROR, &e) == 0)
		return (0);
	return (e.status);
}

int	hs_one(t_wsrv *s, t_handle *c, uint32_t *id)
{
	hs_start(s, 320, 240);
	*c = hr_client(s);
	*id = hs_create(s, *c, rect_make(20, 20, 200, 150), WS_DEFAULT);
	hr_drain(*c, 0, NULL);
	return (hs_slot(s, *id));
}
