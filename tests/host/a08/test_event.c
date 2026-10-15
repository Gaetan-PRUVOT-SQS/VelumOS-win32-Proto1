#include "harness.h"
#include "fake.h"

static void	event_syscalls(void)
{
	t_process	*p;
	int64_t		h;
	t_handle	w;

	fk_reset();
	p = fk_proc_new(0);
	g_fk.cur = p;
	h_eq_i64("manual 2", fk_call(SYS_EVENT_CREATE, 2, 0, 0), E_INVAL);
	h_eq_i64("initial 2", fk_call(SYS_EVENT_CREATE, 0, 2, 0), E_INVAL);
	h = fk_call(SYS_EVENT_CREATE, 0, 0, 0);
	h_eq_i64("op 3", fk_call(SYS_EVENT_OP, h, 3, 0), E_INVAL);
	h_eq_i64("set", fk_call(SYS_EVENT_OP, h, EV_SET, 0), 0);
	h_eq_i64("attente", fk_call(SYS_WAIT, h, 0, 0), 0);
	handle_dup_as(p, (t_handle)h, HR_WAIT, &w);
	h_eq_i64("sans HR_SIGNAL", fk_call(SYS_EVENT_OP, w, EV_SET, 0), E_PERM);
	h_eq_i64("timer_set evenement", fk_call(SYS_TIMER_SET, h, 1, 0), E_BADF);
	h_eq_i64("timer manual 2", fk_call(SYS_TIMER_CREATE, 2, 0, 0), E_INVAL);
	h_eq_i64("dup bits inconnus", fk_call(SYS_DUP, h, 0x200, 0), E_INVAL);
	h_eq_i64("dup handle 0", fk_call(SYS_DUP, 0, 1, 0), E_BADF);
	h_true(fk_call(SYS_DUP, h, HR_WAIT, 0) > 0, "dup reduit");
	h_eq_i64("close", fk_call(SYS_CLOSE, h, 0, 0), 0);
	h_eq_i64("double close", fk_call(SYS_CLOSE, h, 0, 0), E_BADF);
	fk_proc_free(p);
	h_eq_i64("tas rendu", fk_heap_total(), 0);
}

static void	timer_exhaustion(void)
{
	t_object	*t[TMR_SLOTS + 1];
	int			i;

	fk_reset();
	i = 0;
	while (i < TMR_SLOTS && tmr_create(0, &t[i]) == 0)
		i++;
	h_eq_i64("512 minuteries", i, TMR_SLOTS);
	h_eq_i64("513e refusee", tmr_create(0, &t[TMR_SLOTS]), E_NOMEM);
	obj_unref(t[7]);
	h_eq_i64("case rendue", tmr_create(0, &t[7]), 0);
	while (i > 0)
		obj_unref(t[--i]);
	h_eq_i64("tas rendu", fk_heap_total(), 0);
}

static void	no_process(void)
{
	fk_reset();
	g_fk.cur = NULL;
	h_eq_i64("sans processus", fk_call(SYS_EVENT_CREATE, 0, 0, 0), E_INVAL);
	h_eq_i64("listen", fk_call(SYS_PORT_LISTEN, fk_uptr("a"), 1, 0), E_PERM);
	h_eq_i64("connect", fk_call(SYS_PORT_CONNECT, fk_uptr("a"), 1, 0), E_PERM);
	h_eq_i64("wait", fk_call(SYS_WAIT, 5000, 0, 0), E_BADF);
	h_eq_i64("close 0", fk_call(SYS_CLOSE, 0, 0, 0), E_BADF);
	h_eq_i64("section", fk_call(SYS_SECTION_CREATE, 1, PROT_R, 0), E_INVAL);
	h_eq_i64("numero inconnu", fk_call(0x18, 0, 0, 0), E_NOSYS);
	h_eq_i64("tas rendu", fk_heap_total(), 0);
}

int	main(void)
{
	h_begin("a08/event");
	h_run("event_syscalls", event_syscalls);
	h_run("timer_exhaustion", timer_exhaustion);
	h_run("no_process", no_process);
	return (h_end());
}
