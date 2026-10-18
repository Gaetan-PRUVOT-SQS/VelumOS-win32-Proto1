#include "vfs_int.h"
#include "vfs_weak.h"
#include "velum/boot.h"
#include "velum/err.h"
#include "velum/heap.h"
#include "velum/klog.h"
#include "velum/libk.h"

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

#define BPB_LABEL_OFF 71
#define BPB_LABEL_LEN 11
#define BPB_FSTYPE_OFF 82
#define BPB_FSTYPE_LEN 8

typedef enum e_label
{
	LABEL_ILLISIBLE,
	LABEL_DATA,
	LABEL_ESP,
	LABEL_AUTRE
}	t_label;

static t_label	classer_secteur(const uint8_t *s)
{
	if (s[510] != 0x55 || s[511] != 0xaa)
		return (LABEL_ILLISIBLE);
	if (memcmp(s + BPB_FSTYPE_OFF, "FAT32   ", BPB_FSTYPE_LEN) != 0)
		return (LABEL_ILLISIBLE);
	if (memcmp(s + BPB_LABEL_OFF, "VELUMDATA  ", BPB_LABEL_LEN) == 0)
		return (LABEL_DATA);
	if (memcmp(s + BPB_LABEL_OFF, "ESP        ", BPB_LABEL_LEN) == 0)
		return (LABEL_ESP);
	return (LABEL_AUTRE);
}

static t_label	label_du_volume(t_blkdev *d)
{
	uint8_t	*sec;
	t_label	label;

	if (blk_read == NULL || d->sector_size < 512 || d->nsectors == 0)
		return (LABEL_ILLISIBLE);
	sec = kmalloc_tag(d->sector_size, HEAP_FS);
	if (sec == NULL)
		return (LABEL_ILLISIBLE);
	label = LABEL_ILLISIBLE;
	if (blk_read(d, 0, 1, sec) == 0)
		label = classer_secteur(sec);
	kfree(sec);
	return (label);
}

static bool	monter_si(t_blkdev *d, t_label voulu, bool tout_sauf_esp)
{
	t_label	label;

	label = label_du_volume(d);
	if (label == LABEL_ILLISIBLE || label == LABEL_ESP)
		return (false);
	if (!tout_sauf_esp && label != voulu)
		return (false);
	if (vfs_mount("/data", "fat32", d, 0) < 0)
		return (false);
	klog_info("vfs: /data monté sur %.16s", d->name);
	return (true);
}

static bool	monter_passe(uint32_t n, bool tout_sauf_esp)
{
	t_blkdev	*d;
	uint32_t	i;

	i = 0;
	while (i < n)
	{
		d = blk_at(i);
		if (d && monter_si(d, LABEL_DATA, tout_sauf_esp))
			return (true);
		i++;
	}
	return (false);
}

static void	mount_data(void)
{
	uint32_t	n;

	if (blk_count == NULL || blk_at == NULL)
	{
		klog_info("vfs: pas de périphérique bloc, /data absent");
		return ;
	}
	n = blk_count();
	if (monter_passe(n, false) || monter_passe(n, true))
		return ;
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
