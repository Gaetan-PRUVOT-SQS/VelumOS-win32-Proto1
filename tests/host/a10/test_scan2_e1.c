#include "harness.h"
#include "th.h"

static const uint8_t	g_tail[7] = {0x14, 0x77, 0xe1, 0xf0, 0x14, 0xf0, 0x77};

static void	mismatch_at_each_position(void)
{
	t_scan2		s;
	t_keyraw	ev[4];
	uint8_t		seq[9];
	int			p;
	int			n;

	p = 0;
	while (p < 7)
	{
		scan2_reset(&s);
		g_h.name = "transition E1 : octet faux a la position p";
		seq[0] = 0xe1;
		memcpy(&seq[1], g_tail, (size_t)p);
		seq[1 + p] = 0x1c;
		n = th_scan(&s, seq, p + 2, ev);
		h_eq_i64("evenements apres rupture", n, (p >= 2) + 1);
		h_eq_u64("derniere frappe reprise", ev[n - 1].code, 0x1c);
		h_eq_u64("retour au repos", s.state, S2_IDLE);
		p++;
	}
}

static void	press_only_then_normal_key(void)
{
	static const uint8_t	half[] = {0xe1, 0x14, 0x77};
	t_scan2					s;
	t_keyraw				ev[2];

	scan2_reset(&s);
	h_eq_i64("pause : appui seul", th_scan(&s, half, 3, ev), 1);
	h_eq_u64("code pause", ev[0].code, 0xe114);
	h_eq_i64("suite attendue E1", th_scan(&s, &g_tail[2], 1, ev), 0);
	h_eq_u64("moitie de relachement en attente", s.state, S2_E1);
	h_eq_i64("rupture : touche reprise", th_scan(&s, &g_tail[0], 1, ev), 1);
	h_eq_u64("touche reprise", ev[0].code, 0x14);
	h_eq_u64("repos", s.state, S2_IDLE);
}

static void	complete_twice(void)
{
	static const uint8_t	pause[] = {0xe1, 0x14, 0x77, 0xe1, 0xf0, 0x14,
		0xf0, 0x77};
	t_scan2					s;
	t_keyraw				ev[4];

	scan2_reset(&s);
	h_eq_i64("deux pauses d'affilee", th_scan(&s, pause, 8, ev), 2);
	h_eq_u64("repos apres pause", s.state, S2_IDLE);
	h_eq_i64("seconde pause", th_scan(&s, pause, 8, ev), 2);
	h_eq_u64("appui puis relachement", ev[1].release, 1);
	h_eq_u64("pas de repetition", ev[0].repeat, 0);
}

static void	invalid_prefix_transitions(void)
{
	static const uint8_t	e0e0[] = {0xe0, 0xe0, 0x75};
	static const uint8_t	f0f0[] = {0xf0, 0xf0, 0x1c};
	static const uint8_t	e0e1[] = {0xe0, 0xe1, 0x14, 0x77};
	t_scan2					s;
	t_keyraw				ev[2];

	scan2_reset(&s);
	h_eq_i64("E0 E0 : reprise", th_scan(&s, e0e0, 3, ev), 1);
	h_eq_u64("E0 E0 code", ev[0].code, 0xe075);
	scan2_reset(&s);
	h_eq_i64("F0 F0 : relachement", th_scan(&s, f0f0, 3, ev), 1);
	h_eq_u64("F0 F0 release", ev[0].release, 1);
	scan2_reset(&s);
	h_eq_i64("E0 E1 : pause", th_scan(&s, e0e1, 4, ev), 1);
	h_eq_u64("E0 E1 code", ev[0].code, 0xe114);
}

int	main(void)
{
	h_begin("a10/scan2_e1");
	h_run("E1 : rupture a chaque position", mismatch_at_each_position);
	h_run("E1 : appui seul", press_only_then_normal_key);
	h_run("E1 : deux sequences completes", complete_twice);
	h_run("prefixes invalides", invalid_prefix_transitions);
	return (h_end());
}
