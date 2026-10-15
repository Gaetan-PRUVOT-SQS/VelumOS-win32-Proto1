#include "harness.h"
#include "th.h"
#include "velum/err.h"

static void	fifo_order(void)
{
	t_inpevent	out;
	uint32_t	i;

	fake_all_reset();
	h_eq_i64("pop file vide (arme la file noyau)", input_pop(&out), 0);
	th_push_n(INP_KEY_DOWN, 100, 3);
	i = 0;
	while (i < 3)
	{
		h_eq_i64("pop", input_pop(&out), 1);
		h_eq_u64("ordre FIFO", out.code, 100 + i);
		h_eq_u64("type conserve", out.type, INP_KEY_DOWN);
		i++;
	}
	h_eq_i64("vide a nouveau", input_pop(&out), 0);
	h_eq_i64("pop sans destination", input_pop(NULL), E_INVAL);
	input_push(NULL);
	h_eq_i64("verrous equilibres", g_fsync.held + g_fsync.errors, 0);
}

static void	unarmed_cursor_keeps_nothing(void)
{
	t_inpevent	out;

	fake_all_reset();
	th_push_n(INP_CHAR, 1, 5);
	h_eq_i64("rien retenu avant le premier pop", input_pop(&out), 0);
	th_push_n(INP_CHAR, 10, 1);
	h_eq_i64("retenu apres armement", input_pop(&out), 1);
	h_eq_u64("evenement recu", out.code, 10);
	h_eq_u64("aucune perte comptee", input_lost(), 0);
}

static void	timestamps(void)
{
	t_inpevent	ev;
	t_inpevent	out;

	fake_all_reset();
	input_pop(&out);
	ev = th_event(INP_KEY_DOWN, 1);
	input_push(&ev);
	input_pop(&out);
	h_true(out.time_ns != 0, "horodatage ajoute si absent");
	ev.time_ns = 12345;
	input_push(&ev);
	input_pop(&out);
	h_eq_u64("horodatage fourni conserve", out.time_ns, 12345);
}

int	main(void)
{
	h_begin("a10/queue");
	h_run("ordre FIFO et arguments", fifo_order);
	h_run("file noyau non armee", unarmed_cursor_keeps_nothing);
	h_run("horodatage", timestamps);
	return (h_end());
}
