#include <stdlib.h>
#include <string.h>
#include "help.h"

int	hv_check(const void *msg, uint32_t len)
{
	void	*copy;
	int		r;

	copy = malloc(len + (len == 0));
	if (copy == NULL)
		return (E_NOMEM);
	memcpy(copy, msg, len);
	r = wsp_validate(copy, len, 0);
	free(copy);
	return (r);
}

void	hv_hello(t_wmhello *m)
{
	memset(m, 0, sizeof(*m));
	ws_hdr(&m->h, WMC_HELLO, sizeof(*m), 0);
	m->version = WM_VERSION;
}

t_handle	hr_client(t_wsrv *s)
{
	t_handle	c;

	c = hr_connect(s);
	hr_hello(s, c);
	return (c);
}

void	hs_point(t_wsrv *s, int32_t x, int32_t y)
{
	fk_push_input(INP_MOUSE_MOVE, 0, x - s->m.pos.x, y - s->m.pos.y);
	hs_input(s);
}
