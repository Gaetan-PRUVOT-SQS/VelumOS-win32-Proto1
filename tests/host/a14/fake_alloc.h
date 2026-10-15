#ifndef FAKE_ALLOC_H
# define FAKE_ALLOC_H

# include <stddef.h>
# include <stdint.h>

# define FA_HOOK_MAX 16
# define FA_SLOTS 64

typedef struct s_fa_slot
{
	void		*p;
	size_t		n;
	uint8_t		seed;
}	t_fa_slot;

typedef struct s_fa_job
{
	uint64_t	seed;
	uint32_t	ops;
	uint32_t	errors;
	t_fa_slot	slots[FA_SLOTS];
}	t_fa_job;

void		fa_hook_install(void);
void		fa_hook_reset(void);
int			fa_hook_count(void);
int			fa_hook_code(int i);
void		fa_fill(void *p, size_t n, uint8_t seed);
int			fa_intact(const void *p, size_t n, uint8_t seed);
uint64_t	fa_rng(uint64_t *state);
void		fa_job_step(t_fa_job *j);
void		fa_job_drain(t_fa_job *j);
void		*fa_job_thread(void *arg);

#endif
