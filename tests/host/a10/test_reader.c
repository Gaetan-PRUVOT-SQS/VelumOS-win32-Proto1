#include "harness.h"
#include "th.h"

static void	history_not_delivered(void)
{
	t_reader	*r;
	uint32_t	codes[8];

	fake_all_reset();
	th_push_n(INP_CHAR, 1, 3);
	h_true(th_open(INPUT_KIND_ALL, &r) != 0, "ouverture");
	h_eq_i64("rien de l'historique", th_take_codes(r, codes, 8), 0);
	th_push_n(INP_CHAR, 10, 2);
	h_eq_i64("deux nouveaux evenements", th_take_codes(r, codes, 8), 2);
	h_eq_u64("premier", codes[0], 10);
	h_eq_u64("second", codes[1], 11);
}

static void	independent_cursors(void)
{
	t_reader	*a;
	t_reader	*b;
	uint32_t	codes[8];

	fake_all_reset();
	th_open(INPUT_KIND_ALL, &a);
	th_open(INPUT_KIND_ALL, &b);
	h_true(a != b, "deux lecteurs distincts");
	th_push_n(INP_KEY_DOWN, 1, 3);
	h_eq_i64("A lit 3", th_take_codes(a, codes, 8), 3);
	h_eq_i64("A ne relit rien", th_take_codes(a, codes, 8), 0);
	th_push_n(INP_KEY_DOWN, 4, 1);
	h_eq_i64("A lit le nouveau", th_take_codes(a, codes, 8), 1);
	h_eq_i64("B lit toujours 4", th_take_codes(b, codes, 8), 4);
	h_eq_u64("B : ordre", codes[3], 4);
}

static const uint32_t	g_types[] = {INP_KEY_DOWN, INP_CHAR, INP_MOUSE_MOVE,
	INP_MOUSE_DOWN, INP_WHEEL, INP_KEY_UP};
static const uint32_t	g_kinds[] = {INPUT_KIND_KEYBOARD, INPUT_KIND_MOUSE,
	INPUT_KIND_ALL};

static void	kind_filters(void)
{
	t_reader	*r[3];
	uint32_t	codes[8];
	uint32_t	i;

	fake_all_reset();
	i = 0;
	while (i < 3)
	{
		th_open(g_kinds[i], &r[i]);
		i++;
	}
	i = 0;
	while (i < 6)
	{
		th_push_n(g_types[i], i + 1, 1);
		i++;
	}
	h_eq_i64("clavier : 3 evenements", th_take_codes(r[0], codes, 8), 3);
	h_eq_u64("clavier : dernier", codes[2], 6);
	h_eq_i64("souris : 3 evenements", th_take_codes(r[1], codes, 8), 3);
	h_eq_u64("souris : premier", codes[0], 3);
	h_eq_i64("tout : 6 evenements", th_take_codes(r[2], codes, 8), 6);
}

int	main(void)
{
	h_begin("a10/reader");
	h_run("historique non livre", history_not_delivered);
	h_run("curseurs independants", independent_cursors);
	h_run("filtres par genre", kind_filters);
	return (h_end());
}
