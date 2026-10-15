#include "vfs_int.h"
#include "vfs_weak.h"
#include "velum/boot.h"
#include "velum/err.h"
#include "velum/klog.h"

int	vfs_initrd_boot(const char *norm)
{
	const t_bootinfo	*bi;
	uint64_t			base;

	bi = boot_info();
	if (bi == NULL || bi->initrd_size == 0 || bi->initrd_phys == 0)
		return (vfs_initrd_mount(norm, NULL, 0));
	if (__builtin_add_overflow(bi->hhdm, bi->initrd_phys, &base)
		|| base + bi->initrd_size < base)
		return (E_INVAL);
	return (vfs_initrd_mount(norm, (const void *)base, bi->initrd_size));
}

static void	mount_root(void)
{
	int	rc;

	rc = vfs_initrd_boot("/");
	if (rc == 0)
		return ;
	klog_err("vfs: initrd refusé (%d), racine vide", rc);
	rc = vfs_initrd_mount("/", NULL, 0);
	if (rc < 0)
		klog_err("vfs: racine impossible (%d)", rc);
}

static void	mount_data(void)
{
	t_blkdev	*d;
	uint32_t	n;
	uint32_t	i;

	if (blk_count == NULL || blk_at == NULL)
	{
		klog_info("vfs: pas de périphérique bloc, /data absent");
		return ;
	}
	n = blk_count();
	i = 0;
	while (i < n)
	{
		d = blk_at(i);
		if (d && vfs_mount("/data", "fat32", d, 0) == 0)
		{
			klog_info("vfs: /data monté sur %.16s", d->name);
			return ;
		}
		i++;
	}
	klog_info("vfs: aucune partition FAT32, /data absent");
}

int	vfs_boot_init(void)
{
	vfs_state_init();
	mount_root();
	mount_data();
	vfs_sys_register();
	return (0);
}
