#include "proc_int.h"
#include "proc_sys.h"
#include "velum/err.h"
#include "velum/libk.h"

static int	copy_path(const t_sysargs *a, char *path)
{
	uint64_t	len;
	int			rc;

	len = a->a[1];
	if (len == 0 || len >= VFS_PATH_MAX)
		return (E_INVAL);
	rc = copy_from_user(path, a->a[0], len);
	if (rc < 0)
		return (rc);
	path[len] = '\0';
	return (path_check(path, len));
}

static int	copy_args(const t_sysargs *a, char **args)
{
	int	rc;

	*args = NULL;
	if (a->a[3] == 0)
		return (0);
	if (a->a[3] > PROC_ARGS_MAX)
		return (E_INVAL);
	*args = kmalloc_tag(a->a[3], HEAP_PROC);
	if (!*args)
		return (E_NOMEM);
	rc = copy_from_user(*args, a->a[2], a->a[3]);
	if (rc == 0)
		rc = args_count(*args, a->a[3]);
	if (rc >= 0)
		return (0);
	kfree(*args);
	*args = NULL;
	return (rc);
}

static void	spawn_req(t_spawnreq *rq, const t_sysargs *a, const char *path,
				const char *args)
{
	memset(rq, 0, sizeof(*rq));
	rq->path = path;
	rq->args = args;
	rq->args_len = a->a[3];
	rq->flags = (uint32_t)a->a[4];
	rq->parent = proc_current();
}

int64_t	sys_proc_spawn(const t_sysargs *a)
{
	char		path[VFS_PATH_MAX];
	t_spawnreq	rq;
	t_handle	h;
	char		*args;
	int			rc;

	rc = syscall_require(PF_SPAWN);
	if (rc == 0 && (a->a[4] & ~(uint64_t)PF_ALL))
		rc = E_INVAL;
	args = NULL;
	if (rc == 0)
		rc = copy_path(a, path);
	if (rc == 0)
		rc = copy_args(a, &args);
	if (rc < 0)
		return (rc);
	spawn_req(&rq, a, path, args);
	h = HANDLE_INVALID;
	rc = proc_spawn_handle(&rq, &h);
	kfree(args);
	if (rc < 0)
		return (rc);
	return (h);
}

int64_t	sys_proc_kill(const t_sysargs *a)
{
	t_hget		hg;
	t_process	*cur;
	t_process	*target;
	int			rc;

	cur = proc_current();
	target = proc_handle_obj(cur, a->a[0], OBJ_PROCESS, &hg);
	if (!target)
		return (E_BADF);
	rc = 0;
	if (!(hg.rights & HR_SIGNAL) && syscall_require(PF_ADMIN) < 0)
		rc = E_PERM;
	if (rc == 0 && target == cur)
	{
		proc_obj_release(&hg);
		proc_exit_current((int)a->a[1]);
	}
	if (rc == 0)
		proc_kill(target, (int)a->a[1]);
	proc_obj_release(&hg);
	return (rc);
}
