#include "velum/err.h"
#include "velum/libk.h"
#include "velum/settings.h"
#include "velum/wm.h"
#include "fake_theme_boot.h"

#define THEME_PATH_MAX 96

static t_settings	g_loaded;
static int			g_rc = E_NOENT;
static int			g_loads;
static char			g_path[THEME_PATH_MAX];
static int			g_sends;
static uint32_t		g_dark;
static uint32_t		g_reduced;
static uint32_t		g_window;

void	theme_boot_reset(void)
{
	memset(&g_loaded, 0, sizeof(g_loaded));
	g_rc = E_NOENT;
	g_loads = 0;
	g_path[0] = '\0';
	g_sends = 0;
	g_dark = 0;
	g_reduced = 0;
	g_window = 0;
}

void	theme_boot_arm(const t_settings *s, int rc)
{
	memset(&g_loaded, 0, sizeof(g_loaded));
	if (s)
		g_loaded = *s;
	g_rc = rc;
}

static void	fill_default(t_settings *s)
{
	s->theme = SET_THEME_LIGHT;
	s->reduced = 0;
	s->layout = SET_LAYOUT_AZERTY;
	s->mode = 1;
}

int	set_load(const char *path, t_settings *s)
{
	g_loads++;
	g_path[0] = '\0';
	if (path)
		strlcpy(g_path, path, sizeof(g_path));
	if (!s)
		return (E_INVAL);
	if (!path || g_rc < 0)
	{
		fill_default(s);
		if (!path)
			return (E_INVAL);
		return (g_rc);
	}
	*s = g_loaded;
	return (g_rc);
}

bool	set_dark_at(const t_settings *s, uint32_t hour)
{
	if (s->theme == SET_THEME_DARK)
		return (true);
	if (s->theme == SET_THEME_LIGHT)
		return (false);
	return (hour >= SET_NIGHT_FROM || hour < SET_NIGHT_TO);
}

int	wmc_set_theme(const t_wmwin *w, uint32_t dark, uint32_t reduced)
{
	if (w == NULL || w->id == 0 || dark > 1 || reduced > 1)
		return (E_INVAL);
	g_sends++;
	g_dark = dark;
	g_reduced = reduced;
	g_window = w->id;
	return (0);
}

int	theme_boot_loads(void)
{
	return (g_loads);
}

const char	*theme_boot_path(void)
{
	return (g_path);
}

int	theme_boot_sends(void)
{
	return (g_sends);
}

uint32_t	theme_boot_dark(void)
{
	return (g_dark);
}

uint32_t	theme_boot_reduced(void)
{
	return (g_reduced);
}

uint32_t	theme_boot_window(void)
{
	return (g_window);
}
