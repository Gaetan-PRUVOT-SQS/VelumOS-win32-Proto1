#include "proc_int.h"
#include "proc_sys.h"
#include "velum/err.h"
#include "velum/klog.h"
#include "velum/libk.h"

int64_t	sys_proc_info(const t_sysargs *a)
{
	t_procinfo	info;
	t_hget		hg;
	t_process	*p;

	if (a->a[2] < sizeof(info))
		return (E_INVAL);
	hg.obj = NULL;
	p = proc_current();
	if (a->a[0] != 0)
		p = proc_handle_obj(p, a->a[0], OBJ_PROCESS, &hg);
	if (!p)
		return (E_BADF);
	proc_fill_info(p, &info);
	proc_obj_release(&hg);
	return (copy_to_user(a->a[1], &info, sizeof(info)));
}

int64_t	sys_proc_list(const t_sysargs *a)
{
	t_procinfo	*buf;
	uint32_t	max;
	int			n;
	int			rc;

	rc = syscall_require(PF_ADMIN);
	if (rc < 0)
		return (rc);
	max = PROC_MAX;
	if (a->a[1] < max)
		max = (uint32_t)a->a[1];
	if (max == 0)
		return (0);
	buf = kmalloc_tag(sizeof(t_procinfo) * max, HEAP_PROC);
	if (!buf)
		return (E_NOMEM);
	n = proc_list(buf, max);
	rc = copy_to_user(a->a[0], buf, sizeof(t_procinfo) * (uint64_t)n);
	kfree(buf);
	if (rc < 0)
		return (rc);
	return (n);
}

static int	log_allowed(t_process *p)
{
	uint64_t	fl;
	int			ok;

	if (!time_now_ns)
		return (1);
	fl = spin_lock_irqsave(&p->lock);
	ok = rl_allow(&proc_ext(p)->rl, time_now_ns(), SYS_LOG_RATE);
	spin_unlock_irqrestore(&p->lock, fl);
	return (ok);
}

static void	log_emit(const t_process *p, uint64_t level, const char *msg)
{
	if (level == LOG_ERR)
		klog_err("[%u:%s] %s", p->pid, p->name, msg);
	else if (level == LOG_WARN)
		klog_warn("[%u:%s] %s", p->pid, p->name, msg);
	else if (level == LOG_INFO)
		klog_info("[%u:%s] %s", p->pid, p->name, msg);
	else
		klog_debug("[%u:%s] %s", p->pid, p->name, msg);
}

int64_t	sys_log(const t_sysargs *a)
{
	char		buf[SYS_LOG_LEN_MAX + 1];
	t_process	*p;
	uint64_t	len;
	int			rc;

	p = proc_current();
	len = a->a[2];
	if (!p || a->a[0] > LOG_DEBUG || len > SYS_LOG_LEN_MAX)
		return (E_INVAL);
	rc = copy_from_user(buf, a->a[1], len);
	if (rc < 0)
		return (rc);
	buf[len] = '\0';
	log_sanitize(buf, len);
	if (!log_allowed(p))
		return (E_AGAIN);
	log_emit(p, a->a[0], buf);
	return (0);
}
