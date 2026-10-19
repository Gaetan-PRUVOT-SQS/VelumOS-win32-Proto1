#ifndef FAKE_THEME_BOOT_H
# define FAKE_THEME_BOOT_H

# include <stdint.h>
# include "velum/settings.h"

void		theme_boot_reset(void);
void		theme_boot_arm(const t_settings *s, int rc);
int			theme_boot_loads(void);
const char	*theme_boot_path(void);
int			theme_boot_sends(void);
uint32_t	theme_boot_dark(void);
uint32_t	theme_boot_reduced(void);
uint32_t	theme_boot_window(void);

#endif
