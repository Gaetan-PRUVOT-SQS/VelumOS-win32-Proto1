#ifndef TIMER_H
# define TIMER_H

# include <stdbool.h>
# include <stdint.h>

typedef bool	(*t_condfn)(void *ctx);
typedef void	(*t_timerfn)(void *ctx);

int			timer_boot_init(void);
uint64_t	time_now_ns(void);
uint64_t	time_wall_ns(void);
int			time_set_wall_ns(uint64_t ns);
uint64_t	tsc_hz(void);
void		time_delay_ns(uint64_t ns);
int			wait_until(t_condfn cond, void *ctx, uint64_t timeout_ns);
int64_t		timer_arm(uint64_t deadline_ns, t_timerfn fn, void *ctx);
bool		timer_cancel(int64_t id);
int			power_off(void);
int			power_reboot(void);

#endif
