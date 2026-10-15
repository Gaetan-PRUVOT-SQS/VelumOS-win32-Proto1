#include "scan2.h"

static const uint8_t	g_pause_tail[S2_PAUSE_TAIL] = {
	0x14, 0x77, 0xe1, 0xf0, 0x14, 0xf0, 0x77};

int	scan2_e1(t_scan2 *s, uint8_t b, t_keyraw *out)
{
	if (b != g_pause_tail[s->e1pos])
		return (scan2_idle(s, b, out));
	s->e1pos++;
	if (s->e1pos == S2_PAUSE_MAKE)
		return (scan2_emit(s, KEY_PAUSE_CODE, 0, out));
	if (s->e1pos == S2_PAUSE_TAIL)
	{
		s->state = S2_IDLE;
		return (scan2_emit(s, KEY_PAUSE_CODE, 1, out));
	}
	return (0);
}
