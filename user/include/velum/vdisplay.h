#ifndef VDISPLAY_H
# define VDISPLAY_H

# include <stdint.h>
# include "velum/abi/abi_syscall.h"
# include "velum/abi/abi_types.h"

int		v_display_info(t_dispinfo *out);
int64_t	v_display_map(uint64_t hint_va);
int		v_display_set_mode(uint32_t width, uint32_t height, uint32_t bpp);
int		v_display_modes(t_dispmode *out, uint32_t max);
int		v_kcon(int enable);

#endif
