#include "mouse_dec.h"

void	mousedec_reset(t_mousedec *m, int wheel)
{
	m->len = 0;
	m->wheel = (wheel != 0);
	m->last_ns = 0;
}

static int32_t	axis_value(uint8_t low, uint8_t head, uint8_t sign, uint8_t ovf)
{
	int32_t	v;

	if (head & ovf)
		return (0);
	v = low;
	if (head & sign)
		v -= 256;
	return (v);
}

static void	packet_fill(const t_mousedec *m, t_mousepkt *out)
{
	out->dx = axis_value(m->buf[1], m->buf[0], 0x10, 0x40);
	out->dy = axis_value(m->buf[2], m->buf[0], 0x20, 0x80);
	out->dz = 0;
	if (m->wheel)
		out->dz = (int8_t)m->buf[3];
	out->buttons = m->buf[0] & 7;
}

int	mousedec_feed(t_mousedec *m, uint8_t b, uint64_t now, t_mousepkt *out)
{
	uint8_t	need;

	need = 3;
	if (m->wheel)
		need = 4;
	if (m->len > 0 && now > m->last_ns && now - m->last_ns > MD_TIMEOUT_NS)
		m->len = 0;
	m->last_ns = now;
	if (m->len == 0 && !(b & MD_SYNC_BIT))
		return (0);
	m->buf[m->len] = b;
	m->len++;
	if (m->len < need)
		return (0);
	m->len = 0;
	packet_fill(m, out);
	return (1);
}
