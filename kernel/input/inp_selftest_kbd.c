#include "inp_selftest.h"
#include "kbd.h"
#include "mouse.h"
#include "../../drivers/input/mouse_dec.h"
#include "../../drivers/input/scan2.h"

static int	selftest_feed(t_scan2 *s, const uint8_t *seq, int n)
{
	t_keyraw	raw;
	int			events;
	int			i;

	events = 0;
	i = 0;
	while (i < n)
	{
		events += scan2_feed(s, seq[i], &raw);
		i++;
	}
	return (events);
}

int	selftest_scan(void)
{
	static const uint8_t	arrow[] = {0xe0, 0x75, 0xe0, 0xf0, 0x75};
	static const uint8_t	pause[] = {0xe1, 0x14, 0x77, 0xe1, 0xf0, 0x14,
		0xf0, 0x77};
	t_scan2					s;

	scan2_reset(&s);
	if (selftest_feed(&s, arrow, sizeof(arrow)) != 2)
		return (-10);
	if (selftest_feed(&s, pause, sizeof(pause)) != 2)
		return (-11);
	return (0);
}

int	selftest_layout(void)
{
	t_kbd		k;
	t_keyraw	raw;
	t_inpevent	ev[KEY_OUT_MAX];

	kbd_init(&k);
	k.layout = layout_find("fr");
	raw = (t_keyraw){0x15, 0, 0};
	if (kbd_translate(&k, &raw, ev) != 2 || ev[0].code != 'A'
		|| ev[1].code != 'a')
		return (-20);
	raw.code = 0x54;
	if (kbd_translate(&k, &raw, ev) != 1)
		return (-21);
	raw.code = 0x24;
	if (kbd_translate(&k, &raw, ev) != 2 || ev[1].code != 0xea)
		return (-22);
	k.held = KM_RALT;
	raw.code = 0x45;
	if (kbd_translate(&k, &raw, ev) != 2 || ev[1].code != '@')
		return (-23);
	return (0);
}

int	selftest_mouse(void)
{
	static const uint8_t	bytes[] = {0x29, 0x05, 0xfb};
	t_mousedec				d;
	t_mousepkt				pkt;
	t_mouse					m;
	t_inpevent				ev[MOUSE_OUT_MAX];

	mousedec_reset(&d, 0);
	mouse_init(&m);
	if (mousedec_feed(&d, bytes[0], 1, &pkt) || mousedec_feed(&d, bytes[1], 2,
			&pkt) || !mousedec_feed(&d, bytes[2], 3, &pkt))
		return (-30);
	if (mouse_events(&m, &pkt, 0, ev) != 2 || ev[0].x != 5 || ev[0].y != 5)
		return (-31);
	return (0);
}
