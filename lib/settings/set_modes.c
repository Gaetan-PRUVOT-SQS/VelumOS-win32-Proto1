#include "velum/settings.h"

t_setmode	set_mode_get(uint32_t index)
{
	static const t_setmode	modes[SET_MODES] = {{1024, 768}, {1280, 720},
	{1366, 768}, {1600, 900}, {1920, 1080}};

	if (index >= SET_MODES)
		index = 0;
	return (modes[index]);
}

void	set_default(t_settings *s)
{
	s->theme = SET_THEME_LIGHT;
	s->reduced = 0;
	s->layout = SET_LAYOUT_AZERTY;
	s->mode = 1;
}

bool	set_dark_at(const t_settings *s, uint32_t hour)
{
	if (s->theme == SET_THEME_DARK)
		return (true);
	if (s->theme == SET_THEME_LIGHT)
		return (false);
	return (hour >= SET_NIGHT_FROM || hour < SET_NIGHT_TO);
}
