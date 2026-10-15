#include "r3.h"

static void	r3_info_self(t_r3 *t)
{
	t_procinfo	info;
	t_procinfo	*pi;

	pi = &info;
	memset(pi, 0, sizeof(info));
	r3_check(t, "info soi",
		r3_sys(SYS_PROC_INFO, 0, (uint64_t)pi, sizeof(info)), 0);
	r3_check(t, "info soi : pid non nul", info.pid != 0, 1);
	r3_check(t, "info soi : nom ring3test", !strcmp(info.name, "ring3test"),
		1);
	r3_check(t, "info soi : en cours", info.state, 0);
	r3_check(t, "info taille courte",
		r3_sys(SYS_PROC_INFO, 0, (uint64_t)pi, 8), E_INVAL);
	r3_check(t, "info vers le noyau",
		r3_sys(SYS_PROC_INFO, 0, R3_HHDM, sizeof(info)), E_FAULT);
	r3_check(t, "info handle invalide",
		r3_sys(SYS_PROC_INFO, R3_BADH, (uint64_t)pi, sizeof(info)), E_BADF);
}

static void	r3_handles(t_r3 *t)
{
	r3_check(t, "kill handle invalide",
		r3_sys(SYS_PROC_KILL, R3_BADH, 0, 0), E_BADF);
	r3_check(t, "kill handle nul", r3_sys(SYS_PROC_KILL, 0, 0, 0), E_BADF);
	r3_check(t, "kill handle 64 bits",
		r3_sys(SYS_PROC_KILL, 1ull << 40, 0, 0), E_BADF);
	r3_check(t, "prio handle invalide",
		r3_sys(SYS_THREAD_PRIO, R3_BADH, R3_PRIO, 0), E_BADF);
}

static void	r3_prio_thread(t_r3 *t)
{
	uint64_t	entry;

	entry = (uint64_t)r3_thread_tramp;
	r3_check(t, "prio 99", r3_sys(SYS_THREAD_PRIO, 0, 99, 0), E_INVAL);
	r3_check(t, "prio 0", r3_sys(SYS_THREAD_PRIO, 0, 0, 0), E_INVAL);
	r3_check(t, "prio normale", r3_sys(SYS_THREAD_PRIO, 0, R3_PRIO, 0), 0);
	r3_check(t, "fil entrée noyau", r3_thread_raw(R3_KPTR, 0, R3_PRIO, 0),
		E_INVAL);
	r3_check(t, "fil drapeau inconnu", r3_thread_raw(entry, 0, R3_PRIO, 1),
		E_INVAL);
	r3_check(t, "fil pile géante",
		r3_thread_raw(entry, 1ull << 40, R3_PRIO, 0), E_INVAL);
	r3_check(t, "fil prio 40", r3_thread_raw(entry, 0, 40, 0), E_INVAL);
}

static void	r3_memory(t_r3 *t)
{
	t_procinfo	list[8];
	t_procinfo	*pl;
	int64_t		rc;

	pl = list;
	rc = r3_sys(SYS_VALLOC, 0, 4096, PROT_W | PROT_X);
	if (rc == E_NOSYS)
		r3_skip(t, "valloc W|X, appel absent");
	else
		r3_check(t, "valloc W|X refusé", rc, E_INVAL);
	r3_check(t, "liste des processus",
		r3_sys(SYS_PROC_LIST, (uint64_t)pl, 8, 0) >= 1, 1);
	r3_check(t, "liste vers le noyau",
		r3_sys(SYS_PROC_LIST, R3_HHDM, 8, 0), E_FAULT);
}

void	r3_suite_obj(t_r3 *t)
{
	r3_info_self(t);
	r3_handles(t);
	r3_prio_thread(t);
	r3_memory(t);
}
