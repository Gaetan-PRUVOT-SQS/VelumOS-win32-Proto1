#ifndef FAKE_H
# define FAKE_H

# include <stddef.h>
# include <stdint.h>
# include "velum/gfx.h"
# include "velum/apk/droid.h"
# include "apklay.h"

# define FV_FILES 8
# define FV_NAME 96
# define FV_DATA 256
# define FV_READ_STEP 7
# define FV_WRITE_STEP 5

typedef struct s_fvfile
{
	char	name[FV_NAME];
	uint8_t	data[FV_DATA];
	size_t	len;
	size_t	pos;
	int		used;
}	t_fvfile;

typedef struct s_fv
{
	t_fvfile	f[FV_FILES];
	int			calls;
	int			fail_at;
	int			cut;
}	t_fv;

extern t_fv	g_fv;

void		fv_reset(void);
int			fv_tick(void);
int			fv_find(const char *name);
int			fv_put(const char *name, const char *text);
void		fd_reset(t_droid *d);
uint32_t	fd_add(t_droid *d, uint32_t kind, uint32_t parent);
int			fd_inside(const t_apklay *l, t_rect zone);
int			fd_disjoint(const t_apklay *l);

#endif
