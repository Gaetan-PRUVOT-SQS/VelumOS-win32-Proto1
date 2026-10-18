#include <stdlib.h>
#include <string.h>
#include "fake.h"
#include "velum/err.h"
#include "velum/heap.h"
#include "velum/uptr.h"

t_fk	g_fk;

static t_process	g_proc;
static t_object		g_obj;
static int			g_vfile;

uint8_t	fk_pattern(uint64_t i)
{
	return ((uint8_t)((i * 31 + 7) & 0xff));
}

void	fk_reset(uint64_t ro_from)
{
	memset(&g_fk, 0, sizeof(g_fk));
	g_fk.ro_from = ro_from;
	g_fk.rights = HR_READ | HR_WRITE;
	memset(&g_obj, 0, sizeof(g_obj));
	g_obj.type = OBJ_FILE;
	g_obj.impl = &g_vfile;
	g_obj.refs = 1;
}

t_sysargs	*fk_args(uint64_t a0, uint64_t a1, uint64_t a2, uint64_t a3)
{
	static t_sysargs	a;

	memset(&a, 0, sizeof(a));
	a.a[0] = a0;
	a.a[1] = a1;
	a.a[2] = a2;
	a.a[3] = a3;
	return (&a);
}

static bool	fk_inside(t_uptr addr, size_t n)
{
	return (addr >= FK_BASE && addr + n >= addr
		&& addr + n <= FK_BASE + FK_SIZE);
}

bool	user_range_ok(t_uptr addr, size_t n)
{
	if (n > UPTR_COPY_MAX)
		return (false);
	return (fk_inside(addr, n));
}

bool	user_range_wok(t_uptr addr, size_t n)
{
	return (user_range_ok(addr, n) && addr + n <= g_fk.ro_from);
}

int	copy_from_user(void *dst, t_uptr src, size_t n)
{
	if (!user_range_ok(src, n))
		return (E_FAULT);
	memcpy(dst, g_fk.umem + (src - FK_BASE), n);
	return (0);
}

int	copy_to_user(t_uptr dst, const void *src, size_t n)
{
	if (!user_range_wok(dst, n))
		return (E_FAULT);
	memcpy(g_fk.umem + (dst - FK_BASE), src, n);
	return (0);
}

t_process	*proc_current(void)
{
	return (&g_proc);
}

int	handle_get(t_process *p, t_handle h, uint32_t type, t_hget *out)
{
	(void)p;
	if (h != 1 || type != OBJ_FILE)
		return (E_BADF);
	out->obj = &g_obj;
	out->rights = g_fk.rights;
	return (0);
}

void	obj_unref(t_object *obj)
{
	(void)obj;
}

void	*kmalloc_tag(size_t size, t_heap_tag tag)
{
	(void)tag;
	return (malloc(size));
}

void	kfree(void *p)
{
	free(p);
}

int64_t	vfs_xfer(t_vfile *f, t_vio *io, int write, int use_pos)
{
	uint64_t	i;
	uint64_t	n;

	(void)f;
	(void)use_pos;
	if (write)
	{
		i = 0;
		while (i < io->len)
		{
			if (io->src[i] != fk_pattern(g_fk.written + i))
				return (E_IO);
			i++;
		}
		g_fk.written += io->len;
		return ((int64_t)io->len);
	}
	n = io->len;
	if (g_fk.pos + n > FK_FILE)
		n = FK_FILE - g_fk.pos;
	i = 0;
	while (i < n)
	{
		io->dst[i] = fk_pattern(g_fk.pos + i);
		i++;
	}
	g_fk.pos += n;
	return ((int64_t)n);
}
