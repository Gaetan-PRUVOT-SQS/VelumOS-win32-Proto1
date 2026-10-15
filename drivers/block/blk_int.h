#ifndef BLK_INT_H
# define BLK_INT_H

# include <stdbool.h>
# include <stddef.h>
# include <stdint.h>
# include "velum/block.h"
# include "velum/err.h"
# include "velum/heap.h"
# include "velum/klog.h"
# include "velum/libk.h"
# include "velum/sync.h"

# define BLK_MAX_DEVS 64
# define BLK_PARTS_MAX 32
# define BLK_SS_MIN 512
# define BLK_SS_MAX 4096
# define BLK_PART_NUM_MAX 9999
# define EBR_LINKS_MAX 32
# define EBR_FIRST_NUM 5
# define CRC32_INIT 0xffffffffu
# define MBR_TABLE_OFF 446
# define MBR_ENTRY_SIZE 16
# define MBR_ENTRIES 4
# define MBR_SIG_OFF 510
# define MBR_TYPE_GPT 0xee
# define GPT_HDR_MIN 92
# define GPT_REVISION_MAJOR 1
# define GPT_ESZ_MIN 128
# define GPT_ESZ_MAX 4096
# define GPT_ENTRIES_MAX 4096
# define GPT_ARRAY_MAX 262144

typedef struct s_blkreg
{
	t_mutex		lock;
	t_blkdev	*devs[BLK_MAX_DEVS];
	uint32_t	n;
	bool		ready;
}	t_blkreg;

typedef struct s_part
{
	uint64_t	first;
	uint64_t	count;
	uint32_t	num;
}	t_part;

typedef struct s_partlist
{
	t_part		p[BLK_PARTS_MAX];
	uint32_t	n;
	uint32_t	extended;
	bool		gpt;
	uint64_t	ext_first;
	uint64_t	ext_count;
	bool		ext_overlap;
}	t_partlist;

typedef struct s_ebr
{
	uint64_t	ext_first;
	uint64_t	ext_count;
	uint64_t	cur;
	uint64_t	min_next;
	uint32_t	num;
	uint32_t	links;
}	t_ebr;

typedef struct s_gptgeo
{
	uint32_t	ss;
	uint64_t	nsectors;
	uint64_t	lba;
}	t_gptgeo;

typedef struct s_gpthdr
{
	uint64_t	my_lba;
	uint64_t	alt_lba;
	uint64_t	first_usable;
	uint64_t	last_usable;
	uint64_t	entry_lba;
	uint32_t	nentries;
	uint32_t	entry_size;
	uint32_t	array_crc;
	uint32_t	array_bytes;
	uint32_t	array_sectors;
}	t_gpthdr;

static inline uint16_t	rd_le16(const uint8_t *p)
{
	return ((uint16_t)(p[0] | (p[1] << 8)));
}

static inline uint32_t	rd_le32(const uint8_t *p)
{
	return ((uint32_t)p[0] | ((uint32_t)p[1] << 8)
		| ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24));
}

static inline uint64_t	rd_le64(const uint8_t *p)
{
	return ((uint64_t)rd_le32(p) | ((uint64_t)rd_le32(p + 4) << 32));
}

uint32_t	crc32_step(uint32_t state, const void *buf, size_t n);
uint32_t	crc32_calc(const void *buf, size_t n);
t_blkreg	*blk_reg(void);
bool		blk_registered(const t_blkdev *d);
int			blk_dev_valid(const t_blkdev *d);
int			blk_range_ok(const t_blkdev *d, uint64_t lba, uint32_t n);
bool		blk_is_ro(const t_blkdev *d);
int			part_add(t_partlist *pl, const t_part *p);
int			blk_part_name(char *out, const char *disk, uint32_t num);
int			mbr_parse(const uint8_t *sec, uint64_t nsectors, t_partlist *pl);
int			gpt_header(const uint8_t *b, const t_gptgeo *g, t_gpthdr *h);
int			gpt_entries(const uint8_t *a, const t_gpthdr *h, t_partlist *pl);
int			gpt_scan(t_blkdev *d, t_partlist *pl);
int			blk_selftest_write(void);
bool		ebr_is_link(uint8_t type);
int			ebr_note(const uint8_t *e, uint64_t nsectors, t_partlist *pl);
int			ebr_step(const uint8_t *sec, t_ebr *c, t_partlist *pl);
void		ebr_scan(t_blkdev *d, uint8_t *sec, t_partlist *pl);

#endif
