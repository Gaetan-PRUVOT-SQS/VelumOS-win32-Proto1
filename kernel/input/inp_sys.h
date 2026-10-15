#ifndef INP_SYS_H
# define INP_SYS_H

# include <stdint.h>
# include "velum/ksyscall.h"

int64_t	sys_input_open(const t_sysargs *args);
int64_t	sys_input_read(const t_sysargs *args);
int64_t	sys_input_layout(const t_sysargs *args);
int64_t	sys_input_leds(const t_sysargs *args);
int64_t	sys_input_mouse_cfg(const t_sysargs *args);
int		inp_sys_register(void);
int		inp_sys_may_configure(void);

#endif
