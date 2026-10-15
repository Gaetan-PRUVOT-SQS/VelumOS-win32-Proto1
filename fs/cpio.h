#ifndef CPIO_H
# define CPIO_H

# include <stdint.h>
# include "vfs_int.h"

# define CPIO_HDR 110
# define CPIO_FIELDS 13
# define CPIO_MAX_ENTRIES 262144
# define CPIO_TYPE_MASK 0170000
# define CPIO_TYPE_DIR 0040000
# define CPIO_TYPE_REG 0100000
# define FNV_BASIS 0xcbf29ce484222325ull
# define FNV_PRIME 0x100000001b3ull

typedef struct s_cpent
{
	const char		*name;
	const uint8_t	*data;
	uint64_t		size;
	uint64_t		mtime;
	uint32_t		nlen;
	uint32_t		mode;
	uint32_t		order;
	uint32_t		pad;
}	t_cpent;

typedef struct s_cspan
{
	const char	*p;
	uint32_t	n;
	uint32_t	leaf;
}	t_cspan;

typedef struct s_initrd
{
	t_cpent		*ents;
	uint32_t	n;
	uint32_t	skipped;
}	t_initrd;

int			cpio_next(const uint8_t *b, uint64_t size, uint64_t *off,
				t_cpent *e);
int			cpio_name_norm(t_cpent *e);
int			cpio_index(const uint8_t *base, uint64_t size, t_initrd *rd);
void		cpio_sort(t_cpent *v, uint32_t n);
int			cpio_cmp(const char *a, uint32_t an, const char *b, uint32_t bn);
uint32_t	cpio_lower(const t_initrd *rd, const char *key, uint32_t klen);
uint64_t	cpio_hash(uint64_t h, const char *s, uint32_t n);
int			rd_lookup(void *fs, const char *rel, t_vnode *out);
int			rd_open(void *fs, t_vnode *n);
int64_t		rd_read(void *fs, t_vnode *n, t_vio *io);
int64_t		rd_write(void *fs, t_vnode *n, t_vio *io);
int			rd_readdir(void *fs, t_vnode *d, uint64_t *ck, t_dirent *o);
int			rd_create(void *fs, const char *rel, uint32_t t, t_vnode *o);
int			rd_remove(void *fs, const char *rel);
int			rd_rename(void *fs, const char *from, const char *to);
int			rd_truncate(void *fs, t_vnode *n, uint64_t size);
int			rd_sync(void *fs);
void		rd_release(void *fs);

#endif
