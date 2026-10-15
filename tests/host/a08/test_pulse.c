#include "harness.h"
#include "fake.h"

static int	pulse_hook(void *ctx, int forced)
{
	if (!forced)
		return (0);
	evt_op(ctx, EV_PULSE);
	g_fk.hook = NULL;
	return (1);
}

static void	pulse_cases(void)
{
	t_object	*m;
	t_object	*a;

	fk_reset();
	evt_create(1, 1, &m);
	evt_create(0, 0, &a);
	evt_op(m, EV_PULSE);
	h_true(!sig_signaled(m), "pulse remet a zero");
	h_eq_i64("pulse sans attente perdu", obj_wait(m, 0), E_TIMEOUT);
	g_fk.hook = pulse_hook;
	g_fk.hook_ctx = m;
	h_eq_i64("pulse manuel reveille", obj_wait(m, TIMEOUT_INF), 0);
	g_fk.hook = pulse_hook;
	g_fk.hook_ctx = a;
	h_eq_i64("pulse auto reveille", wait_objects(&a, 1, WAIT_ALL,
			TIMEOUT_INF), 0);
	h_true(!sig_signaled(a), "auto reste a zero");
	h_eq_i64("une seule fois", obj_wait(a, 0), E_TIMEOUT);
	obj_unref(m);
	obj_unref(a);
	h_eq_i64("tas rendu", fk_heap_total(), 0);
}

int	main(void)
{
	h_begin("a08/pulse");
	h_run("pulse_cases", pulse_cases);
	return (h_end());
}
