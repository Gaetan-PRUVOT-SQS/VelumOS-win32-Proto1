#include "vfs_int.h"
#include "vfs_weak.h"
#include "velum/heap.h"
#include "velum/libk.h"

static t_vfs	g_vfs;

t_vfs	*vfs_g(void)
{
	if (!g_vfs.ready)
		vfs_state_init();
	return (&g_vfs);
}

void	vfs_state_init(void)
{
	uint32_t	i;

	if (g_vfs.ready)
		return ;
	memset(&g_vfs, 0, sizeof(g_vfs));
	mutex_init(&g_vfs.lock, "vfs.mounts");
	i = 0;
	while (i < VFS_MOUNTS_MAX)
	{
		mutex_init(&g_vfs.mounts[i].lock, "vfs.mount");
		i++;
	}
	g_vfs.ready = 1;
}

uint64_t	vfs_now_ns(void)
{
	if (time_wall_ns == NULL)
		return (0);
	return (time_wall_ns());
}

void	*vfs_alloc(size_t size)
{
	void	*p;

	p = kmalloc_tag(size, HEAP_FS);
	if (p)
		memset(p, 0, size);
	return (p);
}
