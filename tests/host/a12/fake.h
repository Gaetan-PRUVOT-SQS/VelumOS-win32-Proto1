#ifndef FAKE_H
# define FAKE_H

# include <stdint.h>
# include "velum/block.h"
# include "velum/boot.h"
# include "velum/heap.h"
# include "velum/object.h"
# include "velum/proc.h"
# include "vfs_st.h"
# include "velum/ksyscall.h"

# define FAKE_DEVS 4
# define FAKE_HANDLES 512
# define FAKE_KADDR 0xffff800000001000ull
# define FAKE_SEC_MIN 66600
# define FAKE_FSI_FREE 488
# define FAKE_FSI_NEXT 492

typedef struct s_fakeblk
{
	t_blkdev	dev;
	uint8_t		*img;
	uint64_t	oob;
	uint64_t	reads;
	uint64_t	writes;
	int64_t		wfail;
	int64_t		rfail;
	int64_t		watch;
	uint64_t	watch_writes;
	uint32_t	watch_fail;
}	t_fakeblk;

typedef struct s_fakeh
{
	t_object	*obj;
	uint32_t	rights;
	uint32_t	used;
}	t_fakeh;

typedef struct s_fakegeo
{
	uint32_t	bps;
	uint32_t	spc;
	uint32_t	rsvd;
	uint32_t	nfats;
	uint32_t	fatsz;
	uint32_t	nclus;
	uint64_t	tot;
}	t_fakegeo;

typedef struct s_bpbcase
{
	uint32_t	off;
	uint32_t	size;
	uint32_t	val;
	int			want;
	const char	*what;
}	t_bpbcase;

typedef struct s_fsck
{
	const t_fakeblk	*b;
	t_fakegeo		g;
	uint8_t			*used;
	uint32_t		stack[1024];
	uint32_t		top;
	int				errs;
}	t_fsck;

# define MODEL_FILES 24
# define MODEL_MAX 40000

typedef struct s_mfile
{
	char		path[64];
	uint8_t		*data;
	uint32_t	size;
	int			exists;
}	t_mfile;

typedef struct s_model
{
	t_mfile		f[MODEL_FILES];
	uint64_t	seed;
	uint8_t		*buf;
	uint64_t	ops;
	int			errs;
}	t_model;

typedef struct s_fakeheap
{
	int64_t		left;
	int			armed;
	uint64_t	live[HEAP_TAGS];
}	t_fakeheap;

t_fakeblk		*fakeblk_new(uint64_t nsec, uint32_t ssz);
void			fakeblk_free(t_fakeblk *b);
int				fakefat_format(t_fakeblk *b, uint32_t bps, uint32_t spc);
int				fakefat_mount(t_fakeblk *b, const char *at);
void			fakefat_geo(const t_fakeblk *b, t_fakegeo *g);
void			fakefat_bad_from(t_fakeblk *b, uint32_t first);
void			fakefat_set(t_fakeblk *b, uint32_t c, uint32_t v);
uint64_t		fake_heap_live(void);
void			fake_time_set(uint64_t ns);
t_bootinfo		*fake_boot(void);
t_process		*fake_proc(void);
int				fake_close(t_handle h);
t_fakeh			*fake_htab(void);
const t_sysargs	*fake_args(uint64_t a0, uint64_t a1, uint64_t a2, uint64_t a3);
uint32_t		fake_rand(uint64_t *s);
void			fake_append(char *dst, size_t cap, const char *s);
uint8_t			*fake_dup(const void *p, size_t n);
int				fakecpio_valid(t_vbuf *b);
uint64_t		fake_seed(void);
int				fake_populate(const char *root);
int				fake_walk(const char *dir);
void			fake_le(uint8_t *p, uint32_t size, uint32_t v);
int				fake_fsck(const t_fakeblk *b);
uint32_t		fsck_fat(const t_fsck *k, uint32_t c);
uint8_t			*fsck_clus(const t_fsck *k, uint32_t c);
void			fsck_dir(t_fsck *k, uint32_t first);
int				fake_dup_snames(const t_fakeblk *b);
uint8_t			*fake_rootent(t_fakeblk *b, uint32_t idx);
void			model_init(t_model *m, uint64_t seed);
void			model_free(t_model *m);
void			model_step(t_model *m);
int				model_check(t_model *m);
void			model_expect(t_model *m, int64_t got, int64_t want);
void			model_write(t_model *m, t_mfile *f);
void			model_trunc(t_model *m, t_mfile *f);
void			model_rename(t_model *m, t_mfile *a, t_mfile *b);
uint32_t		fake_fsi(const t_fakeblk *b, uint32_t off);
void			fake_fsi_set(t_fakeblk *b, uint32_t off, uint32_t v);
uint32_t		fake_fat_at(const t_fakeblk *b, uint32_t c);
uint32_t		fake_free_real(const t_fakeblk *b);
int64_t			fake_free_gap(const t_fakeblk *b);
uint64_t		fake_warns(void);

#endif
