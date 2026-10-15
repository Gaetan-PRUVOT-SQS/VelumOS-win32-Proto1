#ifndef KINPUT_H
# define KINPUT_H

# include <stdint.h>
# include "abi/abi_input.h"
# include "object.h"

int			input_boot_init(void);
void		input_push(const t_inpevent *ev);
int			input_pop(t_inpevent *ev);
int			input_set_layout(const char *name);
const char	*input_layout(void);
void		input_set_leds(uint32_t mask);
int			input_open(t_process *p, t_handle *out);
int			input_selftest(void);
int			input_open_kind(t_process *p, uint32_t kind, t_handle *out);
int			input_mouse_config(uint32_t speed, uint32_t accel);
uint32_t	input_leds(void);
uint64_t	input_lost(void);

#endif
