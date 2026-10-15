#ifndef FAKE_H
# define FAKE_H

# include <stddef.h>
# include <stdint.h>
# include "velum/err.h"
# include "velum/libk.h"
# include "velum/apk/pm.h"
# include "harness.h"

# define FAKE_FILES 80
# define FAKE_DATA 16384
# define FAKE_SYS "/system/apps"
# define FAKE_USR "/data/apps"
# define FAKE_REG "/data/apps/paquets"
# define FAKE_SYSREG "/system/apps/paquets"

typedef struct s_ffile
{
	int		used;
	char	path[PM_PATH_MAX];
	size_t	len;
	uint8_t	data[FAKE_DATA];
}	t_ffile;

typedef struct s_fakefs
{
	t_ffile	f[FAKE_FILES];
	int		ops;
	int		fail_at;
	int		crash;
}	t_fakefs;

typedef struct s_fakeinsp
{
	t_pminfo	info;
	int			err;
	int			calls;
}	t_fakeinsp;

typedef struct s_fakemem
{
	int	calls;
	int	fail_at;
	int	live;
}	t_fakemem;

typedef struct s_fuzz
{
	t_pmentry	e[8];
	char		text[4096];
	uint8_t		base[4096];
	uint32_t	seed;
}	t_fuzz;

typedef struct s_panne
{
	int		sc;
	int		ops;
	char	old[FAKE_DATA];
	char	neu[FAKE_DATA];
	char	now[FAKE_DATA];
}	t_panne;

extern t_fakefs		g_fs;
extern t_fakeinsp	g_insp;
extern t_fakemem	g_mem;

void			fake_fs_reset(void);
int				fake_fs_tick(void);
t_ffile			*fake_fs_get(const char *path);
int				fake_fs_put(const char *path, const void *data, size_t len);
const t_pmfs	*fake_fs_table(void);
int				fake_inspect(t_span apk, t_pminfo *out);
void			fake_setup(t_pm *pm);
void			fake_app(const char *pkg, uint32_t version, uint8_t signer);
int				fake_install(t_pm *pm, const char *pkg, uint32_t v, uint8_t s);
void			fake_reg_text(char *dst);
int				fake_listed_ok(t_pm *pm);
void			fake_mem_reset(int fail_at);
int				fake_ok(const char *p, const char *v, const char *l,
					const char *a);
char			*fake_rep(char *buf, size_t n, const char *tail);
const char		*fake_hex(void);
int				fake_base(char *dst, size_t cap);

#endif
