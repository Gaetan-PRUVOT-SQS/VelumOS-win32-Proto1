#ifndef FAKE_HOST_H
# define FAKE_HOST_H

# include <setjmp.h>
# include <stdbool.h>
# include <stdint.h>

typedef void	(*t_fhfn)(void *arg);

typedef struct s_fpanic
{
	jmp_buf		jb;
	int			armed;
	void		*owner;
	int			count;
	char		msg[256];
}	t_fpanic;

void		*fh_spawn(t_fhfn fn, void *arg);
void		fh_join(void *h);
void		fh_sleep_us(uint64_t us);
uint64_t	fh_now_ns(void);
void		fh_lock(void);
void		fh_unlock(void);
void		fh_wait_cond(void);
void		fh_broadcast(void);
void		fh_yield(void);
void		*fh_tls_get(void);
void		fh_tls_set(void *p);
void		fake_timer_fail(bool fail);
jmp_buf		*fake_panic_arm(void);
void		fake_panic_disarm(void);
const char	*fake_panic_msg(void);
void		fake_reset_self(void);

#endif
