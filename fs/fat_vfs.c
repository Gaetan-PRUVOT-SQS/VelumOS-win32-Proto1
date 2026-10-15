#include "fat.h"
#include "vfs_weak.h"
#include "velum/err.h"
#include "velum/klog.h"

static const t_fsops	g_fat_ops = {
	"fat32", fat_op_lookup, fat_op_open, fat_op_read, fat_op_write,
	fat_op_readdir, fat_op_create, fat_op_remove, fat_op_rename,
	fat_op_truncate, fat_op_sync, fat_op_release
};

static int	write_info(t_fat *fs)
{
	uint8_t	*b;
	int		rc;

	rc = fat_bget(fs, fs->fsinfo, FC_READ, &b);
	if (rc < 0)
		return (rc);
	if (le32(b) != FAT_FSI_LEAD || le32(b + 484) != FAT_FSI_STRUC)
		return (E_IO);
	put32(b + 488, fs->free_count);
	put32(b + 492, fs->next_free);
	if (!fat_ok(fs, fs->next_free))
		put32(b + 492, FAT_UNKNOWN);
	return (fat_bput(fs, fs->fsinfo, b));
}

int	fat_commit(t_fat *fs, int rc)
{
	if (fs->ro || fs->fsinfo == 0 || !fs->info_dirty)
		return (rc);
	if (write_info(fs) == 0)
	{
		fs->info_dirty = 0;
		return (rc);
	}
	fs->info_fails++;
	if ((fs->info_fails & (fs->info_fails - 1)) == 0)
		klog_warn("vfs: fat32, FSInfo non écrit (%u échecs)",
			fs->info_fails);
	return (rc);
}

int	fat_op_sync(void *fs)
{
	t_fat	*f;
	int		rc;

	f = fs;
	if (f->ro)
		return (0);
	rc = fat_commit(f, 0);
	if (f->fsinfo && f->info_dirty)
		rc = E_IO;
	if (blk_flush && blk_flush(f->dev) < 0 && rc == 0)
		rc = E_IO;
	return (rc);
}

int	vfs_fat_mount(const char *norm, t_blkdev *d, uint32_t fl)
{
	t_fat	*fs;
	int		rc;

	rc = fat_mount(d, fl & VFS_MOUNT_RO, &fs);
	if (rc < 0)
		return (rc);
	if (fs->ro)
		fl |= VFS_MOUNT_RO;
	rc = vfs_mount_add(norm, &g_fat_ops, fs, fl);
	if (rc < 0)
	{
		fat_op_release(fs);
		return (rc);
	}
	klog_info("vfs: fat32 sur %s, %u clusters de %u octets", norm,
		fs->nclus, fs->clsz);
	return (0);
}
