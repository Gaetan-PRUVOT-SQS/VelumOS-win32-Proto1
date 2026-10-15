#ifndef DISPLAY_H
# define DISPLAY_H

# include <stdbool.h>
# include <stddef.h>
# include <stdint.h>
# include "abi/abi_types.h"

# define DISP_FMT_XRGB8888 1

int					display_boot_init(void);
const t_dispinfo	*display_info(void);
void				*display_fb(void);
uint64_t			display_fb_phys(void);
int					display_set_mode(uint32_t w, uint32_t h, uint32_t bpp);
uint32_t			display_modes(t_dispmode *out, uint32_t max);
void				display_boot_progress(uint32_t pct, const char *stage);
void				display_boot_done(void);
void				kcon_write(const char *s, size_t n);
void				kcon_enable(bool on);
bool				kcon_enabled(void);
void				kcon_clear(void);
void				kcon_blue_screen(const char *title, const char *msg);
int					display_selftest(void);

# define DISP_FLAG_WC 0x1
# define DISP_MODE_CURRENT 0x1

#endif
