#ifndef A02_FAKE_H
# define A02_FAKE_H

# include <stdarg.h>
# include <stddef.h>
# include <stdint.h>
# include "harness.h"
# include "pmm_int.h"

# define MIB 0x100000ull
# define GIB 0x40000000ull
# define FAKE_WINDOW_MAX 0x2000000000ull
# define FAKE_POISON 0xab
# define FAKE_DEFAULT_SEED 0x5eed2024ull

typedef struct s_mreq
{
	uint64_t	n;
	uint64_t	al;
	uint64_t	max;
}	t_mreq;

typedef struct s_farg
{
	uint64_t	phys;
	uint64_t	n;
	t_pmm_owner	owner;
}	t_farg;

typedef struct s_snap
{
	uint64_t	*bits;
	uint8_t		*owner;
	t_pmm_stats	stats;
}	t_snap;

# define MODEL_LIVE 4096

typedef struct s_run
{
	t_farg		live[MODEL_LIVE];
	uint64_t	nlive;
	uint64_t	frames;
	uint64_t	allocs;
	uint64_t	frees;
	uint64_t	refused;
	uint64_t	errors;
}	t_run;

typedef struct s_fakeirq
{
	int		depth;
	int		calls;
	int		hook_at;
	int		hook_fired;
	int		relax_calls;
	int		relax_release;
	void	(*hook)(void);
}	t_fakeirq;

extern t_bootinfo	g_fake_info;
extern int			g_corrupt_effective;
extern t_fakeirq	g_fakeirq;
extern uint8_t		*g_model;
extern uint64_t		g_model_span;

int			fake_window_map(uint64_t top);
void		fake_window_poison(void);
void		fake_window_drop(void);
void		fake_reset(void);
void		fake_range(uint64_t base, uint64_t length, uint32_t type);
int			fake_boot(void);
int			fake_region_machine(void);
int			fake_simple(uint64_t first_mib, uint64_t last_mib);
int			fake_catch(void (*fn)(void *), void *arg);
int			fake_free_try(uint64_t phys, uint64_t n, t_pmm_owner o);
const char	*fake_assert_msg(void);
int			fake_assert_count(void);
int			fake_irq_depth(void);
void		fake_hook_arm(int at_call, void (*fn)(void));
int			fake_hook_fired(void);
void		fake_hook_clear(void);
void		fake_relax_arm(void);
int			fake_relax_calls(void);
const char	*fake_last_log(void);
int			fake_log_count(void);
void		fake_log_reset(void);
void		fake_log_capture(const char *fmt, va_list ap);
void		fake_seed(uint64_t seed);
uint64_t	fake_seed_value(void);
uint64_t	fake_rand(void);
uint64_t	fake_below(uint64_t bound);
uint64_t	fake_drain(t_pmm_owner o, uint64_t *out, uint64_t cap);
uint64_t	fake_drain_to(t_pmm_owner o, uint64_t *l, uint64_t c, uint64_t m);
uint64_t	fake_mixed_alloc(uint64_t i);
uint64_t	fake_burst(uint64_t *list);
uint64_t	fake_churn(uint64_t *l, uint64_t *nl, uint64_t t, uint64_t ops);
void		fake_report(const char *label, uint64_t scanned, uint64_t calls);
void		fake_cmdline(const char *fault_mode);
void		fake_snap_take(t_snap *s);
int			fake_snap_same(const t_snap *s);
void		fake_snap_drop(t_snap *s);
uint64_t	fake_random_fill(t_farg *blk, uint64_t cap);
void		fake_shuffle_free(t_farg *blk, uint64_t n);
void		fake_run_track(t_run *r, uint64_t phys, uint64_t n, t_pmm_owner o);
void		fake_run_alloc(t_run *r);
void		fake_run_free(t_run *r);
void		fake_model_run(t_run *r, uint64_t ops);
int			fake_model_machine(void);
void		fake_model_drain(t_run *r);
int			fake_ref_bit(const uint64_t *map, uint64_t i);
uint64_t	fake_ref_diff(const uint64_t *m, uint64_t a, uint64_t b, int in);
uint64_t	fake_ref_next(const uint64_t *m, uint64_t f, uint64_t t, int one);
void		fake_corrupt_free(void);
void		fake_corrupt_owned(void);
void		fake_corrupt_bit(void);
void		fake_corrupt_disarm(void);
void		fake_corrupt_owner(void);
int			fake_corrupt_took_effect(void);
void		fake_corrupt_reset(void);
void		fake_free_list(const uint64_t *list, uint64_t n, t_pmm_owner o);
uint64_t	fake_free_pages(void);
uint64_t	fake_owned(t_pmm_owner o);
uint64_t	fake_list_max(const uint64_t *list, uint64_t n);
uint64_t	fake_list_min(const uint64_t *list, uint64_t n);
uint64_t	fake_list_dups(const uint64_t *list, uint64_t n);
uint64_t	fake_inside(const uint64_t *l, uint64_t n, uint64_t a, uint64_t b);
int			model_init(void);
void		model_done(void);
uint64_t	model_count(int mark);
uint64_t	model_diff(void);
int			model_allowed(uint64_t frame, uint64_t max);
int			model_alloc_ok(const t_mreq *rq, uint64_t phys, t_pmm_owner o);
int			model_feasible(const t_mreq *rq);
int			model_free(uint64_t phys, uint64_t n, t_pmm_owner o);

#endif
