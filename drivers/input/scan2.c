#include "scan2.h"

void	scan2_reset(t_scan2 *s)
{
	s->state = S2_IDLE;
	s->e1pos = 0;
	s->last = 0;
	s->skip = 0;
}

int	scan2_emit(t_scan2 *s, uint16_t code, uint8_t release, t_keyraw *out)
{
	out->code = code;
	out->release = release;
	out->repeat = 0;
	if (release)
	{
		if (s->last == code)
			s->last = 0;
		return (1);
	}
	if (s->last == code)
		out->repeat = 1;
	s->last = code;
	return (1);
}

int	scan2_idle(t_scan2 *s, uint8_t b, t_keyraw *out)
{
	s->state = S2_IDLE;
	if (b == 0xe0)
		s->state = S2_E0;
	else if (b == 0xe1)
	{
		s->state = S2_E1;
		s->e1pos = 0;
	}
	else if (b == 0xf0)
		s->state = S2_BRK;
	else if (b > 0 && b <= S2_MAX_CODE)
		return (scan2_emit(s, b, 0, out));
	return (0);
}

static int	scan2_prefixed(t_scan2 *s, uint8_t b, t_keyraw *out)
{
	int	ext;
	int	brk;

	ext = (s->state == S2_E0 || s->state == S2_E0BRK);
	brk = (s->state == S2_BRK || s->state == S2_E0BRK);
	if (b == 0xe0 || b == 0xe1)
		return (scan2_idle(s, b, out));
	if (b == 0xf0)
	{
		s->state = S2_BRK;
		if (ext)
			s->state = S2_E0BRK;
		return (0);
	}
	s->state = S2_IDLE;
	if (b == 0 || b > S2_MAX_CODE || (ext && (b == 0x12 || b == 0x59)))
		return (0);
	if (ext)
		return (scan2_emit(s, KEY_EXT_PREFIX | b, (uint8_t)brk, out));
	return (scan2_emit(s, b, (uint8_t)brk, out));
}

int	scan2_feed(t_scan2 *s, uint8_t b, t_keyraw *out)
{
	if (b == 0x00 || b == 0xff)
	{
		s->state = S2_IDLE;
		return (0);
	}
	if (s->state == S2_IDLE)
		return (scan2_idle(s, b, out));
	if (s->state == S2_E1)
		return (scan2_e1(s, b, out));
	return (scan2_prefixed(s, b, out));
}
