#include "r3.h"

static int	r3_noperm(void)
{
	int	bad;

	bad = 0;
	if (r3_sys(SYS_PROC_LIST, 0, 0, 0) != E_PERM)
		bad++;
	if (v_spawn(R3_SELF, "exit42", 7, 0) != E_PERM)
		bad++;
	if (r3_sys(SYS_THREAD_PRIO, 0, 20, 0) != E_PERM)
		bad++;
	return (bad);
}

int	r3_child_main(const char *mode)
{
	int	code;

	if (!strcmp(mode, "null"))
		r3_do_null();
	if (!strcmp(mode, "div0"))
		r3_do_div0();
	if (!strcmp(mode, "pile"))
		return (r3_stack_child());
	if (!strcmp(mode, "pileseq"))
		return (r3_seq_child());
	if (!strcmp(mode, "pilefil"))
		return (r3_guard_child());
	if (!strcmp(mode, "exit42"))
		return (42);
	if (!strcmp(mode, "noperm"))
		return (r3_noperm());
	code = r3_entry_child(mode);
	if (code != R3_NO_MODE)
		return (code);
	while (!strcmp(mode, "dort"))
		r3_sys(SYS_SLEEP, R3_NAP_NS, 0, 0);
	return (99);
}

int	main(int argc, char **argv)
{
	t_r3	t;

	if (argc >= 2)
		return (r3_child_main(argv[1]));
	t.ok = 0;
	t.ko = 0;
	t.skip = 0;
	v_log(V_LOG_INFO, "RING3TEST début");
	r3_suite_sys(&t);
	r3_suite_stack(&t);
	r3_suite_spawn(&t);
	r3_suite_obj(&t);
	r3_suite_child(&t);
	r3_suite_thread(&t);
	r3_suite_entry(&t);
	v_logf(V_LOG_INFO, "RING3TEST %d vérifications, %d ignorées", t.ok,
		t.skip);
	if (t.ko == 0)
	{
		v_log(V_LOG_INFO, "RING3TEST PASS");
		return (0);
	}
	v_logf(V_LOG_ERR, "RING3TEST FAIL %d", t.ko);
	return (1);
}
