#ifndef BLOCK_H
# define BLOCK_H

# include <stdint.h>

# define BLK_RO 0x1
# define BLK_REMOVABLE 0x2
# define BLK_NAME_MAX 16

typedef struct s_blkdev	t_blkdev;

typedef struct s_blkops
{
	int	(*read)(t_blkdev *d, uint64_t lba, uint32_t n, void *buf);
	int	(*write)(t_blkdev *d, uint64_t lba, uint32_t n, const void *buf);
	int	(*flush)(t_blkdev *d);
}	t_blkops;

struct s_blkdev
{
	char			name[BLK_NAME_MAX];
	uint32_t		sector_size;
	uint32_t		flags;
	uint64_t		nsectors;
	uint64_t		first_lba;
	t_blkdev		*parent;
	const t_blkops	*ops;
	void			*priv;
};

int			block_boot_init(void);
int			blk_register(t_blkdev *d);
t_blkdev	*blk_find(const char *name);
uint32_t	blk_count(void);
t_blkdev	*blk_at(uint32_t i);
int			blk_read(t_blkdev *d, uint64_t lba, uint32_t n, void *buf);
int			blk_write(t_blkdev *d, uint64_t lba, uint32_t n, const void *buf);
int			blk_flush(t_blkdev *d);
int			blk_scan_partitions(t_blkdev *d);
int			block_selftest(void);

#endif
