#ifndef LT_THEME_PARSE_H
# define LT_THEME_PARSE_H

# include <stdbool.h>
# include <stddef.h>
# include "lt_theme_tab.h"

# define LTH_MAQUETTE "ux design/design_handoff_velumos_glass/velum_theme.h"

bool			lth_parse_line(const char *line, char *name, size_t cap,
					uint32_t *v);
const t_lthv	*lth_find(const char *name);
uint32_t		lth_hex(const char *s);
uint32_t		lth_pct(uint32_t pct, uint32_t rgb);
size_t			lth_count(void);

#endif
