#ifndef FAKE_SYS_H
# define FAKE_SYS_H

# include <setjmp.h>
# include <stddef.h>
# include <stdint.h>

# define FAKE_LOG_LINES 64
# define FAKE_LOG_LEN 256
# define FAKE_NO_EXIT -9999

typedef struct s_fsys
{
	uint64_t	last_num;
	uint64_t	last_args[6];
	uint64_t	calls;
	int64_t		ret;
	int			record;
	int			kernel;
	int64_t		valloc_budget;
	uint64_t	valloc_calls;
	uint64_t	vfree_calls;
	uint64_t	vfree_bad;
	uint64_t	live_maps;
	uint64_t	live_bytes;
	int64_t		vprotect_ret;
	uint64_t	random_calls;
	int			random_fail;
	uint64_t	yield_calls;
	int			exit_armed;
	int64_t		exit_code;
	jmp_buf		exit_jmp;
	int			log_count;
	uint32_t	log_levels[FAKE_LOG_LINES];
	char		log_lines[FAKE_LOG_LINES][FAKE_LOG_LEN];
	uint64_t	threads_alive;
	uint64_t	events_live;
	int			valloc_reuse;
	uint64_t	last_freed;
	uint64_t	last_freed_len;
	int64_t		thread_fail;
	uint64_t	thr_args[5];
}	t_fsys;

extern t_fsys	g_fsys;

int		fake_waiters(uint64_t h);
void	fake_reset(void);
void	fake_kernel_on(void);
int64_t	fake_exit_catch(void (*fn)(void));
void	fake_set_fs(void *tcb);
void	*fake_guarded(size_t bytes);
void	fake_guarded_free(void *p, size_t bytes);

#endif
