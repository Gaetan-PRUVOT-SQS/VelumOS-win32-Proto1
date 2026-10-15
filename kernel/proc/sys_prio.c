#include "proc_int.h"
#include "proc_sys.h"
#include "velum/err.h"
#include "velum/msr.h"

int	syscall_prio_check(uint64_t prio)
{
	if (prio < 1 || prio > PRIO_MAX)
		return (E_INVAL);
	if (prio > PRIO_HIGH && syscall_require(PF_ADMIN) < 0)
		return (E_PERM);
	return (0);
}

int64_t	sys_thread_prio(const t_sysargs *a)
{
	t_hget		hg;
	t_thread	*t;
	int			rc;

	rc = syscall_prio_check(a->a[1]);
	if (rc < 0)
		return (rc);
	if (a->a[0] == 0)
	{
		thread_set_prio(sched_current(), (int)a->a[1]);
		return (0);
	}
	t = proc_handle_obj(proc_current(), a->a[0], OBJ_THREAD, &hg);
	if (!t)
		return (E_BADF);
	if (t->proc != proc_current() && syscall_require(PF_ADMIN) < 0)
		rc = E_PERM;
	else
		thread_set_prio(t, (int)a->a[1]);
	proc_obj_release(&hg);
	return (rc);
}

int64_t	sys_gettid(const t_sysargs *a)
{
	(void)a;
	return (sched_current()->tid);
}

int64_t	sys_set_fsbase(const t_sysargs *a)
{
	t_thread	*t;

	if (a->a[0] >= USER_TOP)
		return (E_INVAL);
	t = sched_current();
	t->fs_base = a->a[0];
	msr_write(MSR_FS_BASE, a->a[0]);
	return (0);
}
