#include "harness.h"
#include "th.h"

static const t_s2case	g_cases[] = {
{"make normale", {0x1c}, 1, 1, {0x1c}, {0}},
{"break normal", {0xf0, 0x1c}, 2, 1, {0x1c}, {1}},
{"make puis break", {0x1c, 0xf0, 0x1c}, 3, 2, {0x1c, 0x1c}, {0, 1}},
{"make etendue fleche haut", {0xe0, 0x75}, 2, 1, {0xe075}, {0}},
{"break etendu fleche haut", {0xe0, 0xf0, 0x75}, 3, 1, {0xe075}, {1}},
{"ctrl droit", {0xe0, 0x14}, 2, 1, {0xe014}, {0}},
{"impr ecran make", {0xe0, 0x12, 0xe0, 0x7c}, 4, 1, {0xe07c}, {0}},
{"limite basse code 0x01", {0x01}, 1, 1, {0x01}, {0}},
{"limite haute code 0x84", {0x84}, 1, 1, {0x84}, {0}},
{"limite haute break 0x84", {0xf0, 0x84}, 2, 1, {0x84}, {1}},
{"limite au dessus 0x85", {0x85}, 1, 0, {0}, {0}},
{"limite etendue 0x7f", {0xe0, 0x7f}, 2, 1, {0xe07f}, {0}},
{"limite etendue invalide 0x90", {0xe0, 0x90}, 2, 0, {0}, {0}},
{"multimedia transmise brute", {0xe0, 0x21}, 2, 1, {0xe021}, {0}},
{"fausse maj relachee", {0xe0, 0xf0, 0x12}, 3, 0, {0}, {0}},
{"fausse maj droite", {0xe0, 0x59}, 2, 0, {0}, {0}},
};

static void	check_case(const t_s2case *c)
{
	t_scan2		s;
	t_keyraw	ev[4];
	int			n;
	int			i;

	g_h.name = c->name;
	scan2_reset(&s);
	n = th_scan(&s, c->in, c->n, ev);
	h_eq_i64("nombre d'evenements", n, c->nev);
	i = 0;
	while (i < n && i < c->nev)
	{
		h_eq_u64("code", ev[i].code, c->code[i]);
		h_eq_u64("relachement", ev[i].release, c->rel[i]);
		i++;
	}
}

static void	run_table(void)
{
	uint32_t	i;

	i = 0;
	while (i < sizeof(g_cases) / sizeof(g_cases[0]))
	{
		check_case(&g_cases[i]);
		i++;
	}
}

static void	ignored_bytes(void)
{
	static const uint8_t	noise[] = {0x00, 0xff, 0xfa, 0xfe, 0xaa, 0xfc,
		0xfd, 0xee};
	t_scan2					s;
	uint32_t				i;

	scan2_reset(&s);
	i = 0;
	while (i < sizeof(noise))
	{
		h_eq_i64("octet de reponse ignore", th_scan(&s, &noise[i], 1, 0), 0);
		h_eq_u64("etat repos", s.state, S2_IDLE);
		i++;
	}
}

static void	repeat_flag(void)
{
	static const uint8_t	seq[] = {0x1c, 0x1c, 0x1c, 0x1d, 0x1d, 0x1c, 0xf0,
		0x1c, 0x1c};
	static const uint8_t	want[] = {0, 1, 1, 0, 1, 0, 0, 0};
	t_scan2					s;
	t_keyraw				ev[9];
	int						i;

	scan2_reset(&s);
	h_eq_i64("nombre", th_scan(&s, seq, 9, ev), 8);
	i = 0;
	while (i < 8)
	{
		h_eq_u64("drapeau repeat", ev[i].repeat, want[i]);
		i++;
	}
}

int	main(void)
{
	h_begin("a10/scan2_basic");
	h_run("table de sequences", run_table);
	h_run("octets de reponse ignores", ignored_bytes);
	h_run("drapeau de repetition", repeat_flag);
	return (h_end());
}
