#include <stdarg.h>
#include <string.h>
#include "fat.h"
#include "fuzz.h"
#include "velum/block.h"
#include "velum/err.h"
#include "velum/heap.h"
#include "velum/klog.h"

#define FAT_FUZZ_BOOT 512u
#define FAT_FUZZ_FSINFO 512u
#define FAT_FUZZ_FAT_SECS 8u
#define FAT_FUZZ_DATA_SECS 32u
#define FAT_FUZZ_SEC 512u
#define FAT_FUZZ_MAX (FAT_FUZZ_BOOT + FAT_FUZZ_FSINFO \
		+ (FAT_FUZZ_FAT_SECS + FAT_FUZZ_DATA_SECS) * FAT_FUZZ_SEC)
#define FAT_FUZZ_ENTRIES 128u
#define FAT_FUZZ_DIRS 24u
#define FAT_FUZZ_DEPTH 3u
#define FAT_FUZZ_READ 2048u
#define FAT_FUZZ_VIRTUAL_SECTORS (1ull << 36)

typedef struct s_fatimg
{
	const uint8_t	*raw;
	size_t			len;
	uint64_t		fsinfo_lba;
	uint64_t		fat_lba;
	uint64_t		data_lba;
}	t_fatimg;

typedef struct s_walkstate
{
	uint32_t	dirs;
	uint32_t	files;
}	t_walkstate;

static t_fatimg	g_img;

uint64_t	vfs_now_ns(void)
{
	return (1760000000ull * 1000000000ull);
}

void	*vfs_alloc(size_t size)
{
	void	*p;

	p = kmalloc_tag(size, HEAP_FS);
	if (p != NULL)
		memset(p, 0, size);
	return (p);
}

int	vfs_mount_add(const char *p, const t_fsops *o, void *s, uint32_t f)
{
	(void)p;
	(void)o;
	(void)s;
	(void)f;
	return (0);
}

void	klog_info(const char *fmt, ...)
{
	(void)fmt;
}

void	klog_warn(const char *fmt, ...)
{
	(void)fmt;
}

int	blk_flush(t_blkdev *d)
{
	(void)d;
	return (0);
}

int	blk_write(t_blkdev *d, uint64_t lba, uint32_t n, const void *buf)
{
	(void)d;
	(void)lba;
	(void)n;
	(void)buf;
	return (E_PERM);
}

static void	copy_window(uint8_t *dst, uint64_t off)
{
	size_t	n;

	memset(dst, 0, FAT_FUZZ_SEC);
	if (off >= g_img.len)
		return ;
	n = g_img.len - (size_t)off;
	if (n > FAT_FUZZ_SEC)
		n = FAT_FUZZ_SEC;
	memcpy(dst, g_img.raw + off, n);
}

static void	read_sector(uint64_t lba, uint8_t *dst)
{
	uint64_t	base;

	base = FAT_FUZZ_BOOT + FAT_FUZZ_FSINFO;
	if (lba == 0)
		copy_window(dst, 0);
	else if (lba == g_img.fsinfo_lba)
		copy_window(dst, FAT_FUZZ_BOOT);
	else if (lba >= g_img.fat_lba && lba - g_img.fat_lba < FAT_FUZZ_FAT_SECS)
		copy_window(dst, base + (lba - g_img.fat_lba) * FAT_FUZZ_SEC);
	else if (lba >= g_img.data_lba
		&& lba - g_img.data_lba < FAT_FUZZ_DATA_SECS)
		copy_window(dst, base + (FAT_FUZZ_FAT_SECS
				+ (lba - g_img.data_lba)) * FAT_FUZZ_SEC);
	else
		memset(dst, 0, FAT_FUZZ_SEC);
}

int	blk_read(t_blkdev *d, uint64_t lba, uint32_t n, void *buf)
{
	uint32_t	i;

	if (n == 0 || lba >= d->nsectors || n > d->nsectors - lba)
		return (E_IO);
	i = 0;
	while (i < n)
	{
		read_sector(lba + i, (uint8_t *)buf + (size_t)i * FAT_FUZZ_SEC);
		i++;
	}
	return (0);
}

static void	locate(const uint8_t *data, size_t size)
{
	uint8_t		boot[FAT_FUZZ_BOOT];
	uint64_t	ratio;
	uint64_t	units;

	memset(boot, 0, sizeof(boot));
	memcpy(boot, data, size < sizeof(boot) ? size : sizeof(boot));
	ratio = le16(boot + 11) / FAT_FUZZ_SEC;
	if (ratio == 0 || ratio > 8)
		ratio = 1;
	units = le32(boot + 36);
	if (units == 0)
		units = le16(boot + 22);
	g_img.raw = data;
	g_img.len = size;
	g_img.fsinfo_lba = (uint64_t)le16(boot + 48) * ratio;
	g_img.fat_lba = (uint64_t)le16(boot + 14) * ratio;
	g_img.data_lba = ((uint64_t)le16(boot + 14) + (uint64_t)boot[16] * units)
		* ratio;
}

static void	check_dirent(const t_dirent *o)
{
	FUZZ_REQUIRE(o->namelen > 0 && o->namelen <= VFS_NAME_MAX);
	FUZZ_REQUIRE(strlen(o->name) == o->namelen);
	FUZZ_REQUIRE(strcmp(o->name, ".") != 0 && strcmp(o->name, "..") != 0);
	FUZZ_REQUIRE(strchr(o->name, '/') == NULL);
	FUZZ_REQUIRE(o->type == S_TYPE_DIR || o->type == S_TYPE_REG);
}

static void	read_file(t_fat *fs, const char *path)
{
	uint8_t	buf[FAT_FUZZ_READ];
	t_vnode	n;
	t_vio	io;
	int64_t	got;

	if (fat_op_lookup(fs, path, &n) != 0 || !(n.mode & S_TYPE_REG))
		return ;
	FUZZ_REQUIRE(n.size <= FAT_FILE_MAX);
	io.dst = buf;
	io.src = NULL;
	io.len = sizeof(buf);
	io.off = 0;
	got = fat_op_read(fs, &n, &io);
	FUZZ_REQUIRE(got <= (int64_t)sizeof(buf));
	io.off = n.size;
	got = fat_op_read(fs, &n, &io);
	FUZZ_REQUIRE(got <= 0);
}

static void	walk_dir(t_fat *fs, const char *path, uint32_t depth,
		t_walkstate *st);

static void	visit(t_fat *fs, const char *path, const t_dirent *o,
		uint32_t depth, t_walkstate *st)
{
	char	child[VFS_PATH_MAX];
	size_t	n;

	n = strlen(path);
	if (n + 1 + o->namelen + 1 > sizeof(child))
		return ;
	memcpy(child, path, n);
	if (n > 0)
		child[n++] = '/';
	memcpy(child + n, o->name, o->namelen + 1);
	if (o->type == S_TYPE_REG && st->files < FAT_FUZZ_ENTRIES)
	{
		st->files++;
		read_file(fs, child);
	}
	else if (o->type == S_TYPE_DIR && st->dirs < FAT_FUZZ_DIRS
		&& depth < FAT_FUZZ_DEPTH)
	{
		st->dirs++;
		walk_dir(fs, child, depth + 1, st);
	}
}

static void	walk_dir(t_fat *fs, const char *path, uint32_t depth,
		t_walkstate *st)
{
	t_vnode		d;
	t_dirent	o;
	uint64_t	ck;
	uint32_t	count;

	if (fat_op_lookup(fs, path, &d) != 0 || !(d.mode & S_TYPE_DIR))
		return ;
	ck = 0;
	count = 0;
	while (count < FAT_FUZZ_ENTRIES && fat_op_readdir(fs, &d, &ck, &o) == 1)
	{
		check_dirent(&o);
		visit(fs, path, &o, depth, st);
		count++;
	}
}

static void	check_geometry(const t_fat *fs, uint64_t device_bytes)
{
	FUZZ_REQUIRE(fs->bps >= 512 && fs->bps <= 4096);
	FUZZ_REQUIRE(fs->spc > 0 && (fs->spc & (fs->spc - 1)) == 0);
	FUZZ_REQUIRE(fs->clsz == fs->bps * fs->spc);
	FUZZ_REQUIRE(fs->nclus >= FAT_MIN_CLUS && fs->nclus <= FAT_MAX_CLUS);
	FUZZ_REQUIRE(fs->tot_sec * fs->bps <= device_bytes);
	FUZZ_REQUIRE(fs->data_start + (uint64_t)fs->nclus * fs->spc
		<= fs->tot_sec);
	FUZZ_REQUIRE(fs->fat_start + (uint64_t)fs->nfats * fs->fat_sz
		<= fs->data_start);
	FUZZ_REQUIRE(fs->active < fs->nfats);
	FUZZ_REQUIRE(fs->root >= 2 && fs->root < fs->nclus + 2);
	FUZZ_REQUIRE(fs->fsinfo < fs->rsvd);
}

int	LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
	static const t_blkops	ops = {blk_read, blk_write, blk_flush};
	t_blkdev				dev;
	t_walkstate				st;
	t_fat					*fs;
	int						k;

	if (size > FAT_FUZZ_MAX)
		return (0);
	locate(data, size);
	memset(&dev, 0, sizeof(dev));
	dev.sector_size = FAT_FUZZ_SEC;
	dev.nsectors = FAT_FUZZ_VIRTUAL_SECTORS;
	dev.flags = BLK_RO;
	dev.ops = &ops;
	k = 0;
	while (k < 3)
	{
		fuzz_heap_fail_after(k++);
		FUZZ_REQUIRE(fat_mount(&dev, 1, &fs) < 0);
	}
	fuzz_heap_fail_after(-1);
	if (fat_mount(&dev, 1, &fs) == 0)
	{
		check_geometry(fs, dev.nsectors * dev.sector_size);
		memset(&st, 0, sizeof(st));
		walk_dir(fs, "", 0, &st);
		fat_op_release(fs);
	}
	FUZZ_REQUIRE(fuzz_heap_live() == 0);
	return (0);
}
