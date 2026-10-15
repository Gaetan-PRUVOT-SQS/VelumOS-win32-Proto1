#include "harness.h"
#include "th.h"

static const uint32_t	g_pushes[] = {0, 1, 255, 256, 257, 300, 1000};

static void	check_overflow(uint32_t n)
{
	t_inpevent	out;
	uint32_t	first;
	uint32_t	kept;

	fake_all_reset();
	input_pop(&out);
	th_push_n(INP_WHEEL, 0, n);
	first = 0;
	if (n > INPQ_LEN)
		first = n - INPQ_LEN;
	h_eq_u64("pertes comptees", input_lost(), first);
	kept = 0;
	while (input_pop(&out) > 0)
	{
		h_eq_u64("evenement le plus ancien conserve", out.code, first + kept);
		kept++;
	}
	h_eq_u64("evenements conserves", kept, n - first);
}

static void	overflow_boundaries(void)
{
	uint32_t	i;

	g_h.name = "file pleine : limites 0 1 255 256 257 300 1000";
	i = 0;
	while (i < sizeof(g_pushes) / sizeof(g_pushes[0]))
	{
		check_overflow(g_pushes[i]);
		i++;
	}
}

static void	steady_flow(void)
{
	t_inpevent	out;
	uint32_t	i;

	fake_all_reset();
	input_pop(&out);
	i = 0;
	while (i < 10000)
	{
		th_push_n(INP_KEY_UP, i, 1);
		h_eq_i64("un pour un", input_pop(&out), 1);
		h_eq_u64("valeur", out.code, i);
		i++;
	}
	h_eq_u64("aucune perte en flux consomme", input_lost(), 0);
	h_eq_i64("verrous equilibres", g_fsync.held + g_fsync.errors, 0);
}

int	main(void)
{
	h_begin("a10/queue_full");
	h_run("file pleine : valeurs limites", overflow_boundaries);
	h_run("flux consomme sans perte", steady_flow);
	return (h_end());
}
