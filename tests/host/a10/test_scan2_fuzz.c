#include "harness.h"
#include "th.h"

static int	valid_event(const t_keyraw *e)
{
	if (e->release > 1 || e->repeat > 1)
		return (0);
	if (e->code == 0xe114)
		return (1);
	if ((e->code & 0xff00) == 0xe000)
		return ((e->code & 0xff) >= 1 && (e->code & 0xff) <= 0x84);
	return (e->code >= 1 && e->code <= 0x84);
}

static void	random_bytes(void)
{
	t_scan2		s;
	t_keyraw	raw;
	uint64_t	rng;
	int			i;
	int			bad;

	rng = th_seed("a10/scan2_fuzz");
	scan2_reset(&s);
	bad = 0;
	i = 0;
	while (i < 100000)
	{
		if (scan2_feed(&s, (uint8_t)th_rand(&rng), &raw) && !valid_event(&raw))
			bad++;
		if (s.state > S2_E1)
			bad++;
		i++;
	}
	h_eq_i64("evenements mal formes sur 100000 octets", bad, 0);
}

static const uint8_t	g_frame[] = {0x1c, 0xf0, 0x1c, 0xe0, 0x75};
static const uint8_t	g_reset[] = {0x00};
static const uint8_t	g_junk[] = {0, 0xe1, 0xe0, 0xf0, 0xe0, 0xf0, 0xe1,
	0x14};

static void	resync_after_garbage(void)
{
	uint8_t		junk[8];
	uint64_t	rng;
	t_scan2		s;
	t_keyraw	ev[3];
	int			round;

	rng = th_seed("a10/scan2_resync");
	round = 0;
	while (round < 500)
	{
		scan2_reset(&s);
		memcpy(junk, g_junk, sizeof(junk));
		junk[0] = (uint8_t)th_rand(&rng);
		th_scan(&s, junk, 8, 0);
		th_scan(&s, g_reset, 1, 0);
		h_eq_i64("evenements de la trame", th_scan(&s, g_frame, 5, ev), 3);
		h_eq_u64("derniere touche", ev[2].code, 0xe075);
		round++;
	}
}

int	main(void)
{
	h_begin("a10/scan2_fuzz");
	h_run("100000 octets aleatoires", random_bytes);
	h_run("resynchronisation apres bruit", resync_after_garbage);
	return (h_end());
}
