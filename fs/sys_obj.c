#include "fsys.h"
#include "vfs_weak.h"
#include "velum/err.h"

static void	fobj_destroy(t_object *obj)
{
	t_vfile	*f;

	f = obj->impl;
	if (f == NULL)
		return ;
	obj->impl = NULL;
	fsys_quota_drop(f->owner);
	vfs_close(f);
}

static bool	fobj_signaled(t_object *obj)
{
	(void)obj;
	return (true);
}

static const t_objops	g_file_ops = {"file", fobj_destroy, fobj_signaled};

static uint32_t	file_rights(const t_vfile *f)
{
	uint32_t	rt;
	uint32_t	acc;

	rt = HR_WAIT | HR_DUP | HR_TRANSFER;
	acc = f->flags & VFS_ACC_MASK;
	if (acc != O_WRONLY)
		rt |= HR_READ;
	if (acc != O_RDONLY)
		rt |= HR_WRITE;
	return (rt);
}

int	fsys_handle_new(t_process *p, t_vfile *f, t_handle *out)
{
	t_object	*o;
	uint32_t	type;
	int			rc;

	type = OBJ_FILE;
	if (f->node->mode & S_TYPE_DIR)
		type = OBJ_DIR;
	o = obj_create(type, &g_file_ops, f);
	if (o == NULL)
	{
		fsys_quota_drop(f->owner);
		vfs_close(f);
		return (E_NOMEM);
	}
	rc = handle_alloc(p, o, file_rights(f), out);
	obj_unref(o);
	return (rc);
}
