#include "proc_int.h"
#include "velum/boot.h"
#include "velum/err.h"
#include "velum/klog.h"
#include "velum/libk.h"

int	proc_boot_init(void)
{
	t_proctab	*tab;
	int			rc;

	tab = proc_tab();
	spin_init(&tab->lock, "proctab");
	tab->next_pid = 1;
	rc = reap_init();
	if (rc < 0)
		return (rc);
	if (cpu_set_user_fault)
		cpu_set_user_fault(proc_fault);
	else
		klog_warn("proc: crochet des fautes utilisateur absent");
	__atomic_store_n(&tab->ready, 1, __ATOMIC_RELEASE);
	klog_info("proc: %u processus, %u fils par processus, pile %u Kio",
		PROC_MAX, PROC_THREADS_MAX, (unsigned)(USTACK_SIZE >> 10));
	return (0);
}

static int	init_report(const char *path, int rc, t_process *p)
{
	if (rc == E_NOENT)
	{
		klog_info("init: %s absent, le noyau continue sans init", path);
		return (0);
	}
	if (rc < 0)
	{
		klog_err("init: lancement de %s impossible (%d)", path, rc);
		return (rc);
	}
	proc_tab()->init_pid = p->pid;
	klog_info("init: %s lancé (pid %u)", path, p->pid);
	proc_unref(p);
	return (0);
}

int	init_start(void)
{
	t_spawnreq	rq;
	t_process	*p;
	const char	*path;
	char		args[BOOT_ARGS_CAP];
	int			rc;

	path = boot_cmdline_get("init");
	if (!path)
		path = "/system/bin/init";
	if (!__atomic_load_n(&proc_tab()->ready, __ATOMIC_ACQUIRE))
	{
		klog_warn("init: processus indisponibles, %s non lancé", path);
		return (0);
	}
	memset(&rq, 0, sizeof(rq));
	rq.path = path;
	rq.args = args;
	rq.args_len = boot_args_init(boot_info()->cmdline, args, sizeof(args));
	rq.flags = PF_ALL;
	p = NULL;
	rc = proc_spawn(&rq, &p);
	return (init_report(path, rc, p));
}
