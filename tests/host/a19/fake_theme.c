#include "fake.h"
#include "velum/luna.h"

static t_lunatheme	g_theme;
static bool			g_reduced;

void	luna_theme_set(t_lunatheme theme)
{
	g_theme = theme;
}

t_lunatheme	luna_theme_get(void)
{
	return (g_theme);
}

void	luna_transparency_set(bool reduced)
{
	g_reduced = reduced;
}

bool	luna_transparency_reduced(void)
{
	return (g_reduced);
}
