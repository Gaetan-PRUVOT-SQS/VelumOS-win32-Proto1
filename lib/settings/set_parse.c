#include "velum/err.h"
#include "velum/libk.h"
#include "velum/settings.h"

static const char	*g_theme_names[] = {"clair", "sombre", "auto"};
static const char	*g_layout_names[] = {"azerty", "qwerty"};
static const char	*g_transp_names[] = {"normale", "reduite"};
static const char	*g_mode_names[SET_MODES] = {"1024x768", "1280x720",
	"1366x768", "1600x900", "1920x1080"};

static int	index_of(const char *const *names, size_t n, const char *v,
	size_t len)
{
	size_t	i;

	i = 0;
	while (i < n)
	{
		if (strlen(names[i]) == len && memcmp(names[i], v, len) == 0)
			return ((int)i);
		i++;
	}
	return (-1);
}

static bool	apply(t_settings *s, const char *k, size_t kl, const char *v)
{
	size_t	vl;
	int		i;

	vl = strlen(v);
	if (kl == 5 && memcmp(k, "theme", 5) == 0)
		i = index_of(g_theme_names, 3, v, vl);
	else if (kl == 12 && memcmp(k, "transparence", 12) == 0)
		i = index_of(g_transp_names, 2, v, vl);
	else if (kl == 7 && memcmp(k, "clavier", 7) == 0)
		i = index_of(g_layout_names, 2, v, vl);
	else if (kl == 10 && memcmp(k, "resolution", 10) == 0)
		i = index_of(g_mode_names, SET_MODES, v, vl);
	else
		return (false);
	if (i < 0)
		return (false);
	if (kl == 5)
		s->theme = (uint32_t)i;
	else if (kl == 12)
		s->reduced = (uint32_t)i;
	else if (kl == 7)
		s->layout = (uint32_t)i;
	else
		s->mode = (uint32_t)i;
	return (true);
}

static bool	parse_line(t_settings *s, const char *line, size_t n)
{
	char		val[16];
	const char	*eq;
	size_t		vl;

	if (n && line[n - 1] == '\r')
		n--;
	eq = memchr(line, '=', n);
	if (!eq)
		return (false);
	vl = n - (size_t)(eq - line) - 1;
	if (vl == 0 || vl >= sizeof(val))
		return (false);
	memcpy(val, eq + 1, vl);
	val[vl] = '\0';
	return (apply(s, line, (size_t)(eq - line), val));
}

int	set_parse(const char *text, size_t len, t_settings *s)
{
	size_t		pos;
	size_t		n;
	const char	*nl;
	int			ok;

	if (!s || (!text && len))
		return (E_INVAL);
	set_default(s);
	if (len > SET_FILE_MAX)
		return (E_RANGE);
	pos = 0;
	ok = 0;
	while (pos < len)
	{
		nl = memchr(text + pos, '\n', len - pos);
		n = nl ? (size_t)(nl - (text + pos)) : len - pos;
		if (parse_line(s, text + pos, n))
			ok++;
		pos += n + 1;
	}
	return (ok);
}

static bool	add(char *out, size_t max, size_t *pos, const char *t)
{
	size_t	n;

	n = strlen(t);
	if (*pos + n >= max)
		return (false);
	memcpy(out + *pos, t, n);
	*pos += n;
	out[*pos] = '\0';
	return (true);
}

int	set_format(const t_settings *s, char *out, size_t max)
{
	size_t	pos;

	if (!s || !out || max == 0 || s->theme > SET_THEME_AUTO || s->reduced > 1
		|| s->layout > SET_LAYOUT_QWERTY || s->mode >= SET_MODES)
		return (E_INVAL);
	pos = 0;
	out[0] = '\0';
	if (!add(out, max, &pos, "theme=") || !add(out, max, &pos,
			g_theme_names[s->theme]) || !add(out, max, &pos,
			"\ntransparence=") || !add(out, max, &pos, g_transp_names[s->reduced])
		|| !add(out, max, &pos, "\nclavier=") || !add(out, max, &pos,
			g_layout_names[s->layout]) || !add(out, max, &pos, "\nresolution=")
		|| !add(out, max, &pos, g_mode_names[s->mode])
		|| !add(out, max, &pos, "\n"))
		return (E_RANGE);
	return ((int)pos);
}
