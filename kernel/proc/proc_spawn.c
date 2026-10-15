#include "proc_int.h"
#include "velum/err.h"
#include "velum/libk.h"

static int	spawn_check(t_spawnctx *c)
{
	const t_spawnreq	*rq;
	const char			*src;

	rq = c->rq;
	if (!rq->path && !rq->image)
		return (E_INVAL);
	if (rq->flags & ~(uint32_t)PF_ALL)
		return (E_INVAL);
	c->nargs = args_count(rq->args, rq->args_len);
	if (c->nargs < 0)
		return (c->nargs);
	src = rq->path;
	if (!src)
		src = "/image";
	c->path_len = strnlen(src, VFS_PATH_MAX);
	if (path_check(src, c->path_len) < 0)
		return (E_INVAL);
	memcpy(c->path, src, c->path_len);
	c->path[c->path_len] = '\0';
	c->flags = rq->flags;
	if (rq->parent)
		c->flags &= rq->parent->flags;
	return (0);
}

static int	spawn_image(t_spawnctx *c)
{
	void	*buf;
	size_t	size;
	int		rc;

	if (c->rq->image)
	{
		c->img = c->rq->image;
		c->size = c->rq->image_size;
		return (0);
	}
	if (!vfs_read_all)
		return (E_NOENT);
	buf = NULL;
	size = 0;
	rc = vfs_read_all(c->path, &buf, &size);
	if (rc < 0)
		return (rc);
	c->owned = buf;
	c->img = buf;
	c->size = size;
	return (0);
}

static void	spawn_identity(t_process *p, const t_spawnctx *c)
{
	spin_init(&p->lock, "proc");
	p->refs = 1;
	p->flags = c->flags;
	p->state = PS_RUNNING;
	p->mem_limit = PROC_MEM_DEFAULT;
	if (c->rq->parent)
		p->ppid = c->rq->parent->pid;
	proc_name_from_path(p->name, c->path, c->path_len);
	if (time_now_ns)
		p->created_ns = time_now_ns();
}

static int	spawn_new(t_spawnctx *c)
{
	t_process	*p;

	p = kmalloc_tag(sizeof(t_procbox), HEAP_PROC);
	if (!p)
		return (E_NOMEM);
	memset(p, 0, sizeof(t_procbox));
	c->p = p;
	spawn_identity(p, c);
	p->aspace = vmm_aspace_create();
	if (!p->aspace)
		return (E_NOMEM);
	if (!handle_table_create)
		return (0);
	p->handles = handle_table_create();
	if (!p->handles)
		return (E_NOMEM);
	return (0);
}

int	spawn_run(t_spawnctx *c)
{
	int	rc;

	rc = spawn_check(c);
	if (rc == 0)
		rc = spawn_image(c);
	if (rc == 0)
		rc = elf_validate(c->img, c->size, &c->info);
	if (rc == 0)
		rc = spawn_new(c);
	if (rc == 0)
		rc = proc_load_image(c);
	if (rc == 0)
		rc = proc_setup_stack(c);
	if (rc == 0)
		rc = proc_start_main(c);
	if (rc < 0 && c->p)
		proc_abort(c->p);
	if (rc < 0 && c->out)
		*c->out = NULL;
	if (c->owned)
		kfree(c->owned);
	return (rc);
}
