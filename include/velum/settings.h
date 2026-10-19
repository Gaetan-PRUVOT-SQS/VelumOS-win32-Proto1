#ifndef SETTINGS_H
# define SETTINGS_H

# include <stdbool.h>
# include <stddef.h>
# include <stdint.h>

# define SET_THEME_LIGHT 0
# define SET_THEME_DARK 1
# define SET_THEME_AUTO 2
# define SET_LAYOUT_AZERTY 0
# define SET_LAYOUT_QWERTY 1
# define SET_FILE_MAX 512
# define SET_NIGHT_FROM 19
# define SET_NIGHT_TO 7
# define SET_MODES 5
# define SET_PATH "/data/reglages"

typedef struct s_setmode
{
	uint32_t	w;
	uint32_t	h;
}	t_setmode;

typedef struct s_settings
{
	uint32_t	theme;
	uint32_t	reduced;
	uint32_t	layout;
	uint32_t	mode;
}	t_settings;

void		set_default(t_settings *s);
int			set_parse(const char *text, size_t len, t_settings *s);
int			set_format(const t_settings *s, char *out, size_t max);
bool		set_dark_at(const t_settings *s, uint32_t hour);
t_setmode	set_mode_get(uint32_t index);
int			set_load(const char *path, t_settings *s);
int			set_save(const char *path, const t_settings *s);

#endif
