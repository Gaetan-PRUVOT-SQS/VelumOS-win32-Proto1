#ifndef FAKE_H
# define FAKE_H

# include <stdbool.h>
# include <stddef.h>
# include <stdint.h>
# include "velum/block.h"
# include "velum/pci.h"
# include "virtio.h"

# define FK_ALLOCS 4096
# define FK_DEVS 4
# define FK_BAR 4
# define FK_BAR_SIZE 0x4000
# define FK_COMMON 0x0000
# define FK_ISR 0x1000
# define FK_DEVCFG 0x2000
# define FK_NOTIFY 0x3000
# define FK_REGION 0x1000
# define FK_MULT 4
# define FK_HANG 0x1
# define FK_IOERR 0x2
# define FK_CRAZY_IDX 0x4
# define FK_BAD_ID 0x8
# define FK_SHORT_LEN 0x10
# define FK_NO_STATUS 0x20
# define FK_RESET_STUCK 0x40
# define FK_REFUSE_FEAT 0x80
# define FK_DELAY 0x100
# define FK_GEN_FLAP 0x200
# define FK_ENABLE_FAIL 0x400
# define FK_MAP_FAIL 0x800
# define FK_INTX_FAIL 0x1000
# define FK_POLLS 4
# define FK_RAM_SECTORS 4096
# define FK_LOG 256

typedef struct s_fkmem
{
	void	*ptr[FK_ALLOCS];
	int64_t	fail_after;
	bool	armed;
	int		live;
	int		locks;
	int		timeouts;
	int		warns;
	int		errs;
	char	last[FK_LOG];
	char	cmdline[FK_LOG];
}	t_fkmem;

typedef struct s_fkdev
{
	t_pcidev	pci;
	bool		present;
	uint8_t		cfg[256];
	uint8_t		*bar;
	uint8_t		devcfg[64];
	uint8_t		*disk;
	uint64_t	disk_bytes;
	uint64_t	features;
	uint64_t	drv_features;
	uint64_t	qaddr[3];
	uint32_t	modes;
	uint32_t	dfsel;
	uint32_t	gfsel;
	uint16_t	qmax;
	uint16_t	qsel;
	uint16_t	qsize;
	uint16_t	qenable;
	uint16_t	notify_off;
	uint16_t	last_avail;
	uint8_t		status;
	uint8_t		gen;
	uint16_t	cmd;
	int			intx_calls;
	int			raw_cmd_writes;
	bool		pending;
	int			requests;
	int			flushes;
	uint32_t	last_type;
	uint64_t	last_sector;
}	t_fkdev;

typedef struct s_fkram
{
	t_blkdev	dev;
	uint8_t		*data;
	uint64_t	last_lba;
	uint32_t	last_n;
	int			reads;
	int			writes;
	int			flushes;
	int			fail_rc;
	uint64_t	fail_at;
}	t_fkram;

typedef struct s_fkpart
{
	uint64_t	first;
	uint64_t	last;
	uint32_t	slot;
}	t_fkpart;

typedef struct s_fkcase
{
	const char	*name;
	uint32_t	ss;
	uint64_t	ns;
	uint64_t	first;
	int			ops;
	int			want;
}	t_fkcase;

typedef struct s_fkedit
{
	uint32_t	off;
	uint32_t	width;
	uint64_t	val;
	int			want;
}	t_fkedit;

typedef struct s_fkprobe
{
	uint32_t	modes;
	uint64_t	features;
	uint16_t	qmax;
	uint32_t	cfg_off;
	uint32_t	cfg_val;
	int			want;
}	t_fkprobe;

typedef struct s_fkfault
{
	const char	*what;
	uint32_t	modes;
	int			op;
	int			want;
	bool		dead;
}	t_fkfault;

extern t_fkmem	g_fk;
extern t_fkdev	g_fkdev[FK_DEVS];

void		fk_fail_after(int64_t n);
bool		fk_should_fail(void);
void		*fk_track(void *p);
void		fk_untrack(void *p);
int			fk_live(void);
void		fk_reset_log(void);
void		fk_dev_init(t_fkdev *f, uint64_t sectors, uint16_t device);
void		fk_dev_free(t_fkdev *f);
void		fk_dev_cfg32(t_fkdev *f, uint32_t off, uint32_t v);
void		fk_tick(void);
void		fk_process(t_fkdev *f);
void		fk_common_wr(t_fkdev *f, uint32_t off, uint32_t v);
uint32_t	fk_common_rd(t_fkdev *f, uint32_t off);
uint32_t	fk_region_rd(t_fkdev *f, uint32_t off, uint32_t width);
void		fk_region_wr(t_fkdev *f, uint32_t off, uint32_t v);
void		fk_ram_init(t_fkram *r, const char *nm, uint32_t ss, uint64_t ns);
void		fk_ram_free(t_fkram *r);
void		fk_gpt_build(t_fkram *r, const t_fkpart *p, uint32_t n);
void		fk_gpt_rehash(t_fkram *r, uint64_t lba);
void		fk_vq_serve(t_vq *q, uint16_t *last, int rev);
void		fk_le32(uint8_t *p, uint32_t v);
void		fk_pattern(uint8_t *part, uint32_t n);
void		fk_le64(uint8_t *p, uint64_t v);
t_fkdev		*fk_by_addr(volatile uint8_t *a, uint32_t *off);
void		fk_mbr_entry(uint8_t *s, int slot, uint8_t type, const t_fkpart *p);
void		fk_ebr_sector(uint8_t *s, const t_fkpart *l, const t_fkpart *n);
void		fk_ebr_chain(t_fkram *r, uint64_t ext, uint32_t n, uint32_t stride);

#endif
