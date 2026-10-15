#ifndef FAT_H
# define FAT_H

# include <stdint.h>
# include "vfs_int.h"

# define FAT_EOC 0x0FFFFFFF
# define FAT_EOC_MIN 0x0FFFFFF8
# define FAT_MASK 0x0FFFFFFF
# define FAT_HIGH 0xF0000000
# define FAT_MAX_CLUS 0x0FFFFFF5
# define FAT_MIN_CLUS 65525
# define FAT_DIR_MAX 65536
# define FAT_DENT 32
# define FAT_FILE_MAX 0xFFFFFFFFull
# define FAT_CACHE_SLOTS 32
# define FAT_LFN_MAX 20
# define FAT_LFN_CHARS 13
# define FAT_UNITS 261
# define FAT_A_RO 0x01
# define FAT_A_VOL 0x08
# define FAT_A_DIR 0x10
# define FAT_A_ARC 0x20
# define FAT_A_LFN 0x0F
# define FAT_A_LFN_MASK 0x3F
# define FAT_LFN_LAST 0x40
# define FAT_SLOT_FREE 0xE5
# define FAT_UNKNOWN 0xFFFFFFFF
# define FAT_FSI_LEAD 0x41615252
# define FAT_FSI_STRUC 0x61417272
# define FAT_FSI_TRAIL 0xAA550000
# define FAT_ROOT_INO 1
# define FAT_TAIL_TRIES 512
# define FAT_DEPTH_MAX 256
# define FC_READ 0
# define FC_ZERO 1

typedef struct s_fcslot
{
	uint64_t	sec;
	uint32_t	age;
	uint32_t	valid;
	uint8_t		*buf;
}	t_fcslot;

typedef struct s_flfn
{
	uint16_t	u[FAT_UNITS];
	uint32_t	count;
	uint32_t	next;
	uint32_t	start;
	uint32_t	ok;
	uint32_t	sum;
}	t_flfn;

typedef struct s_fat
{
	t_blkdev	*dev;
	uint64_t	tot_sec;
	uint64_t	fat_start;
	uint64_t	data_start;
	uint32_t	bps;
	uint32_t	spc;
	uint32_t	clsz;
	uint32_t	nclus;
	uint32_t	fat_sz;
	uint32_t	nfats;
	uint32_t	rsvd;
	uint32_t	active;
	uint32_t	mirror;
	uint32_t	root;
	uint32_t	fsinfo;
	uint32_t	free_count;
	uint32_t	next_free;
	uint32_t	ratio;
	uint32_t	ro;
	uint32_t	info_dirty;
	uint32_t	info_fails;
	uint32_t	tick;
	int32_t		xerr;
	uint8_t		*slab;
	uint8_t		*zero;
	t_fcslot	slot[FAT_CACHE_SLOTS];
	t_flfn		lfn;
	uint16_t	u16[FAT_UNITS];
}	t_fat;

typedef struct s_fent
{
	char		name[VFS_NAME_MAX + 1];
	char		sname[13];
	uint8_t		raw[FAT_DENT];
	uint64_t	pos;
	uint32_t	idx;
	uint32_t	first_idx;
}	t_fent;

typedef struct s_fdirit
{
	uint32_t	first;
	uint32_t	clus;
	uint32_t	cidx;
	uint32_t	idx;
	uint32_t	run;
}	t_fdirit;

typedef struct s_fspan
{
	uint64_t		at;
	uint64_t		len;
	uint8_t			*dst;
	const uint8_t	*src;
	uint32_t		write;
}	t_fspan;

typedef struct s_fxfer
{
	t_vio		*io;
	uint64_t	done;
	uint32_t	write;
}	t_fxfer;

typedef struct s_fwalk
{
	uint64_t	len;
	uint32_t	c;
	uint32_t	saved;
	uint32_t	power;
	uint32_t	lam;
}	t_fwalk;

typedef struct s_fadd
{
	const uint16_t	*u;
	uint32_t		ulen;
	uint32_t		nlfn;
	uint32_t		start;
	uint8_t			raw[FAT_DENT];
}	t_fadd;

typedef struct s_frename
{
	t_vnode		pf;
	t_vnode		pt;
	t_fent		src;
	t_fent		dst;
	const char	*nf;
	const char	*nt;
	uint32_t	same;
	uint32_t	isdir;
	uint32_t	sfirst;
}	t_frename;

uint32_t	le16(const uint8_t *p);
uint32_t	le32(const uint8_t *p);
void		put16(uint8_t *p, uint32_t v);
void		put32(uint8_t *p, uint32_t v);
int			fat_ok(const t_fat *fs, uint32_t c);
int			fat_bpb_parse(const uint8_t *bs, uint64_t bytes, uint32_t ss,
				t_fat *fs);
int			fat_bget(t_fat *fs, uint64_t sec, int mode, uint8_t **out);
int			fat_bput(t_fat *fs, uint64_t sec, const uint8_t *buf);
void		fat_cache_drop(t_fat *fs, uint64_t sec, uint64_t n);
uint64_t	fat_clus_sec(const t_fat *fs, uint32_t c);
uint64_t	fat_clus_byte(const t_fat *fs, uint32_t c);
int			fat_dread(t_fat *fs, uint64_t sec, uint32_t n, uint8_t *dst);
int			fat_dwrite(t_fat *fs, uint64_t sec, uint32_t n, const uint8_t *s);
int			fat_zero_clus(t_fat *fs, uint32_t c);
int			fat_get(t_fat *fs, uint32_t c, uint32_t *v);
int			fat_set(t_fat *fs, uint32_t c, uint32_t v);
int			fat_alloc(t_fat *fs, uint32_t *out);
int			fat_free_chain(t_fat *fs, uint32_t c);
int			fat_clus_at(t_fat *fs, t_vnode *n, uint32_t idx, uint32_t *out);
int			fat_clus_new(t_fat *fs, t_vnode *n, uint32_t idx, uint32_t *out);
int			fat_link(t_fat *fs, t_vnode *n, uint32_t c);
int			fat_chain_check(t_fat *fs, uint32_t first, uint64_t need);
int			fat_span(t_fat *fs, t_fspan *sp);
int64_t		fat_xfer(t_fat *fs, t_vnode *n, t_vio *io, int write);
int			fat_fill_zero(t_fat *fs, t_vnode *n, uint64_t to);
void		fat_time_pack(uint64_t ns, uint16_t *date, uint16_t *time);
uint64_t	fat_time_unpack(uint32_t date, uint32_t time);
uint8_t		fat_lfn_sum(const uint8_t *raw);
void		fat_sname_decode(const uint8_t *raw, char *out);
int			fat_name_valid(const char *name);
int			fat_utf8_to16(const char *s, uint16_t *u, uint32_t max);
int			fat_utf16_to8(const uint16_t *u, uint32_t n, char *out,
				uint32_t cap);
void		fat_basis(const char *name, uint8_t *raw, int *lossy);
int			fat_fits_83(const char *name);
void		fat_sname_try(const char *name, const uint8_t *basis, uint32_t k,
				uint8_t *out);
void		fdir_init(t_fdirit *it, uint32_t first);
int			fdir_pos(t_fat *fs, t_fdirit *it, uint32_t idx, uint64_t *pos);
int			fdir_get(t_fat *fs, uint64_t pos, uint8_t *e);
int			fdir_put(t_fat *fs, uint64_t pos, const uint8_t *e);
void		fat_lfn_feed(t_flfn *l, const uint8_t *e, uint32_t idx);
int			fat_lfn_name(const t_flfn *l, const uint8_t *raw, char *out);
void		fat_lfn_build(uint8_t *e, const t_fadd *a, uint32_t ord);
int			fdir_scan(t_fat *fs, t_fdirit *it, t_fent *ent);
int			fdir_find(t_fat *fs, uint32_t first, const char *name, t_fent *o);
int			fdir_empty(t_fat *fs, uint32_t first);
int			fdir_add(t_fat *fs, uint32_t first, const char *name, t_fent *io);
int			fdir_del(t_fat *fs, uint32_t first, const t_fent *ent);
int			fdir_extend(t_fat *fs, t_fdirit *it);
int			fat_pick_short(t_fat *fs, uint32_t first, const char *name,
				t_fadd *a);
int			fat_parent_of(t_fat *fs, uint32_t dir, uint32_t *parent);
int			fat_set_dotdot(t_fat *fs, uint32_t dir, uint32_t parent);
int			fat_mkdir_clus(t_fat *fs, uint32_t parent, const uint8_t *tmpl,
				uint32_t *out);
void		fat_tmpl(uint8_t *raw, uint32_t attr, uint32_t first);
void		fat_root_node(const t_fat *fs, t_vnode *o);
int			fat_node_from(const t_fat *fs, const t_fent *e, t_vnode *o);
int			fat_node_sync(t_fat *fs, t_vnode *n, int touch);
int			fat_parent(t_fat *fs, const char *rel, t_vnode *pn,
				const char **name);
int			fat_mount(t_blkdev *d, uint32_t ro, t_fat **out);
int			fat_op_lookup(void *fs, const char *rel, t_vnode *out);
int			fat_op_open(void *fs, t_vnode *n);
int64_t		fat_op_read(void *fs, t_vnode *n, t_vio *io);
int64_t		fat_op_write(void *fs, t_vnode *n, t_vio *io);
int			fat_op_readdir(void *fs, t_vnode *d, uint64_t *ck, t_dirent *o);
int			fat_op_create(void *fs, const char *rel, uint32_t t, t_vnode *o);
int			fat_op_remove(void *fs, const char *rel);
int			fat_op_rename(void *fs, const char *from, const char *to);
int			fat_op_truncate(void *fs, t_vnode *n, uint64_t size);
int			fat_op_sync(void *fs);
void		fat_op_release(void *fs);
int			fat_commit(t_fat *fs, int rc);

#endif
