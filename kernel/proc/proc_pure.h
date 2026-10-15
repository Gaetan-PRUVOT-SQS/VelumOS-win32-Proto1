#ifndef PROC_PURE_H
# define PROC_PURE_H

# include <stdint.h>

# define PROC_NARGS_MAX 256
# define PROC_NAME_LEN 32
# define USTACK_AUX_PAIRS 8
# define USTACK_RANDOM 16
# define AT_NULL 0
# define AT_PHDR 3
# define AT_PHNUM 5
# define AT_PAGESZ 6
# define AT_ENTRY 9
# define AT_RANDOM 25
# define AT_VELUM_ABI 0x7001
# define AT_VELUM_FLAGS 0x7002
# define RL_WINDOW_NS 1000000000ull

typedef struct s_ratelimit
{
	uint64_t	start_ns;
	uint32_t	count;
	uint32_t	dropped;
}	t_ratelimit;

typedef struct s_ustack
{
	uint8_t		*buf;
	uint64_t	cap;
	uint64_t	top;
	uint64_t	sp;
}	t_ustack;

typedef struct s_ustart
{
	const char		*path;
	uint64_t		path_len;
	const char		*args;
	uint64_t		args_len;
	uint32_t		nargs;
	uint32_t		flags;
	const uint8_t	*rnd;
	uint64_t		entry;
	uint64_t		phdr;
	uint64_t		phnum;
}	t_ustart;

int		args_count(const char *args, uint64_t len);
int		path_check(const char *path, uint64_t len);
void	proc_name_from_path(char *dst, const char *path, uint64_t len);
int		rl_allow(t_ratelimit *rl, uint64_t now_ns, uint32_t max_per_s);
void	log_sanitize(char *s, uint64_t n);
int		ustack_push(t_ustack *st, const void *src, uint64_t n);
int		ustack_put(t_ustack *st, uint64_t va, uint64_t val);
int		ustack_build(t_ustack *st, const t_ustart *in);
int		ustack_align(t_ustack *st, uint64_t n);
int		ustack_vectors(t_ustack *st, const t_ustart *in, uint64_t str_va,
			uint64_t rnd_va);

#endif
