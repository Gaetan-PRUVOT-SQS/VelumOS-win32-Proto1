#ifndef LT_THEME_TAB_H
# define LT_THEME_TAB_H

# include <stddef.h>
# include <stdint.h>

typedef struct s_lthv
{
	const char	*name;
	uint32_t	value;
}	t_lthv;

typedef struct s_lthp
{
	const char	*name;
	uint32_t	value;
	uint32_t	pct;
	uint32_t	rgb;
}	t_lthp;

typedef struct s_lthx
{
	const char	*name;
	uint32_t	value;
	const char	*hex;
}	t_lthx;

typedef struct s_lthf
{
	const char	*name;
	size_t		off;
	uint32_t	pct;
	uint32_t	rgb;
}	t_lthf;

extern const t_lthf	g_lth_light[];
extern const t_lthf	g_lth_dark[];
extern const t_lthv	g_lth_all[];
extern const t_lthp	g_lth_pct[];
extern const t_lthx	g_lth_hex[];

#endif
