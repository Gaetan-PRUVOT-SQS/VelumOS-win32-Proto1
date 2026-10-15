#ifndef R3_H
# define R3_H

# include <stdint.h>
# include <string.h>
# include "velum/velum.h"

# define R3_KPTR 0xffffffff80000000ull
# define R3_HHDM 0xffff800000001000ull
# define R3_BADH 0x7ffffff0ull
# define R3_WAIT_NS 5000000000ull
# define R3_NAP_NS 50000000ull
# define R3_PRIO 8
# define R3_LOG_LEN 200
# define R3_SELF "/system/bin/ring3test"
# define R3_MAGIC 0x1234
# define R3_PAGE 0x1000ull
# define R3_STACK_SIZE 0x100000ull
# define R3_NO_GUARD 77
# define R3_NO_BASE 78
# define R3_NO_FAULT 79
# define R3_POLLS 500
# define R3_TSTACK_SIZE 0x40000ull
# define R3_TSTACK_MIN 0x4000ull
# define R3_USER_MIN 0x10000ull
# define R3_USER_TOP 0x00007ffffffff000ull
# define R3_WALK_MAX 4096
# define R3_THREADS 1000
# define R3_STACK_FREED 76
# define R3_STACK_PROT 75
# define R3_NO_MODE -1000

typedef struct s_r3
{
	int	ok;
	int	ko;
	int	skip;
}	t_r3;

int64_t		r3_sys(uint64_t num, uint64_t a0, uint64_t a1, uint64_t a2);
void		r3_check(t_r3 *t, const char *name, int64_t got, int64_t want);
void		r3_skip(t_r3 *t, const char *name);
int64_t		r3_spawn_raw(uint64_t path, uint64_t len, uint64_t args,
				uint64_t alen);
int64_t		r3_child(const char *mode, uint32_t flags);
int64_t		r3_reap(int64_t h, t_procinfo *info);
int64_t		r3_thread_raw(uint64_t entry, uint64_t stack, uint64_t prio,
				uint64_t flags);
void		r3_suite_sys(t_r3 *t);
void		r3_suite_spawn(t_r3 *t);
void		r3_suite_obj(t_r3 *t);
void		r3_suite_child(t_r3 *t);
void		r3_suite_thread(t_r3 *t);
void		r3_suite_stack(t_r3 *t);
int			r3_stack_child(void);
uint64_t	r3_stack_base(uint64_t size);
void		r3_suite_guard(t_r3 *t, uint64_t guard);
int			r3_guard_child(void);
void		r3_ovf_main(void);
void		r3_ovf_tramp(void);
uint64_t	r3_shared(void);
void		r3_hold_tramp(void);
void		r3_hold_main(uint64_t arg);
int			r3_seq_child(void);
void		r3_suite_held(t_r3 *t, uint64_t base);
void		r3_suite_entry(t_r3 *t);
int			r3_entry_child(const char *mode);
int			r3_child_main(const char *mode);
void		r3_do_null(void);
void		r3_do_div0(void);
void		r3_do_overflow(void);
void		r3_thread_tramp(void);
void		r3_thread_main(uint64_t arg);

#endif
