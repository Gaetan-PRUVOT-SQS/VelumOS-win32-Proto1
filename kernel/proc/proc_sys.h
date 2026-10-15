#ifndef PROC_SYS_H
# define PROC_SYS_H

# include <stdint.h>
# include "velum/ksyscall.h"

# define SYSTAB_SIZE 256
# define SYSTAB_LOG_MAX 16
# define SYS_LOG_LEN_MAX 200
# define SYS_LOG_RATE 100

typedef struct s_systab
{
	t_sysfn		fn[SYSTAB_SIZE];
	const char	*name[SYSTAB_SIZE];
	uint64_t	calls[SYSTAB_SIZE];
	uint64_t	unknown;
}	t_systab;

typedef struct s_sysdef
{
	uint32_t	num;
	t_sysfn		fn;
	const char	*name;
}	t_sysdef;

int		proc_sys_register(void);
int64_t	sys_exit(const t_sysargs *a);
int64_t	sys_proc_spawn(const t_sysargs *a);
int64_t	sys_proc_kill(const t_sysargs *a);
int64_t	sys_proc_info(const t_sysargs *a);
int64_t	sys_thread_create(const t_sysargs *a);
int64_t	sys_thread_exit(const t_sysargs *a);
int64_t	sys_thread_prio(const t_sysargs *a);
int64_t	sys_gettid(const t_sysargs *a);
int64_t	sys_set_fsbase(const t_sysargs *a);
int64_t	sys_proc_list(const t_sysargs *a);
int64_t	sys_log(const t_sysargs *a);

#endif
