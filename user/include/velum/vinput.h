#ifndef VINPUT_H
# define VINPUT_H

# include <stddef.h>
# include <stdint.h>
# include "velum/abi/abi_input.h"
# include "velum/abi/abi_syscall.h"
# include "velum/abi/abi_types.h"

# define V_INPUT_ALL 0
# define V_LAYOUT_GET 0
# define V_LAYOUT_SET 1

int64_t	v_input_open(uint32_t kind);
int		v_input_read(t_handle input, t_inpevent *out, uint32_t max);
int		v_input_layout(uint32_t op, char *name, size_t name_len);
int		v_input_leds(uint32_t mask);
int		v_input_mouse_cfg(uint32_t speed, uint32_t accel);

#endif
