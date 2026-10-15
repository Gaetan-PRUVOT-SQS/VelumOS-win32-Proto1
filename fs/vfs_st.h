#ifndef VFS_ST_H
# define VFS_ST_H

# include <stdint.h>
# include "vfs_int.h"

# define VST_PATTERN_LEN 70000

typedef struct s_vbuf
{
	uint8_t		*p;
	uint64_t	len;
	uint64_t	cap;
}	t_vbuf;

int		vst_cpio_add(t_vbuf *b, const char *name, uint32_t mode,
			const char *data);
int		vst_check(int ok, const char *what);
int		vst_put(const char *path, const void *data, uint64_t len);
int		vst_same(const char *path, const void *data, uint64_t len);
int		vst_initrd(void);
int		vst_data(void);

#endif
