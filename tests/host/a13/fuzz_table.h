#ifndef FUZZ_TABLE_H
# define FUZZ_TABLE_H

# include "kfix.h"

# define FZ_SLOTS 256
# define FZ_BUF 4096
# define FZ_WRITABLE 2048
# define FZ_MAPS_MAX 3
# define FZ_REPORT_MAX 20
# define FZ_GOLDEN_PATH "tests/abi/syscalls.golden"
# define FZ_OUTSIDE_COUNT 5

enum e_fzstate
{
	FZ_UNKNOWN = 0,
	FZ_ACTIVE,
	FZ_RETIRED,
	FZ_HOLE
};

typedef struct s_fzcall
{
	uint32_t	num;
	uint64_t	a[6];
	uint32_t	pid;
	uint32_t	flags;
	bool		no_proc;
}	t_fzcall;

typedef struct s_fzrun
{
	uint64_t	seed_start;
	uint64_t	seed;
	uint64_t	calls;
	uint64_t	incidents;
	uint64_t	by_num[FZ_SLOTS];
	uint64_t	nosys;
	uint64_t	outside;
	uint64_t	ok;
	uint64_t	refused;
	uint32_t	registered[FZ_SLOTS];
	uint32_t	registered_count;
	uint8_t		*buf;
	uint8_t		gstate[FZ_SLOTS];
	uint8_t		gowned[FZ_SLOTS];
	uint8_t		gargs[FZ_SLOTS];
	uint32_t	gactive;
}	t_fzrun;

uint32_t	fz_scan(uint32_t *nums);
bool		fz_documented(int64_t rc);
bool		fz_registered(uint32_t num);
void		fz_gen(t_fzrun *r, t_fzcall *c);
int64_t		fz_call(const t_fzcall *c, bool *halted);
const char	*fz_verdict(const t_fzrun *r, const t_fzcall *c, int64_t rc);
void		fz_report(const t_fzrun *r, const t_fzcall *c, int64_t rc,
				const char *invariant);
bool		fz_golden_load(t_fzrun *r);
const char	*fz_check_table(const t_fzrun *r);
bool		fz_sweep(t_fzrun *r);
bool		fz_run(t_fzrun *r, uint64_t calls, uint64_t seconds);
void		fz_open(t_fzrun *r, uint64_t seed, uint8_t *buf);

#endif
