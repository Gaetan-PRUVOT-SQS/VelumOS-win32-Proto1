#ifndef SM_PROGS_H
# define SM_PROGS_H

# include <stddef.h>
# include <stdint.h>

# define SM_PROGS_MAX 3
# define SM_PROGS_FILE_MAX 480
# define SM_PROG_NAME_MAX 32
# define SM_PROG_PATH_MAX 64
# define SM_PROG_ROOT "/system/"
# define SM_PROG_ROOT_LEN 8
# define SM_PROGS_PATH "/system/etc/programmes"
# define SM_PROG_DEFAULT_NAME "Bonjour"
# define SM_PROG_DEFAULT_PATH "/system/bin/hello"

typedef struct s_smprog
{
	char	name[SM_PROG_NAME_MAX];
	char	path[SM_PROG_PATH_MAX];
	char	arg[SM_PROG_PATH_MAX];
}	t_smprog;

typedef struct s_smprogs
{
	uint32_t	count;
	t_smprog	list[SM_PROGS_MAX];
}	t_smprogs;

int		sm_progs_line(t_smprog *p, const char *s, size_t n);
int		sm_progs_parse(const char *buf, size_t len, t_smprogs *out);
void	sm_progs_default(t_smprogs *out);
int		sm_set_programs(const t_smprogs *p);
int32_t	sm_prog_of(uint32_t cmd);

#endif
