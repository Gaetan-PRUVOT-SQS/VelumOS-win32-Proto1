#ifndef PLATFORM_H
# define PLATFORM_H

# include <stddef.h>
# include <stdint.h>
# include "velum/abi/abi_types.h"

uint64_t	os_mono_ns(void);
uint64_t	os_wall_ns(void);
int			os_sleep_ns(uint64_t ns);
int			os_timer_open(t_handle *out);
int			os_timer_after(t_handle timer, uint64_t delay_ns,
				uint64_t period_ns);
int			os_timer_cancel(t_handle timer);
int			os_wait1(t_handle h, uint64_t timeout_ns);
int			os_wait2(t_handle a, t_handle b, uint64_t timeout_ns);
int			os_spawn(const char *path, const char *arg, uint32_t flags,
				t_handle *proc);
int			os_exited(t_handle proc);
int			os_wait_exit(t_handle proc);
void		os_kill(t_handle proc);
void		os_close(t_handle h);
int			os_power(uint32_t op);
int			os_sysinfo(t_sysinfo *out);
void		os_log(const char *msg);
int			os_read_file(const char *path, char *buf, size_t max, size_t *len);

#endif
