#include "apk_int.h"

static int	name_dots(const char *s, int dollar)
{
	int		dots;
	size_t	i;
	char	c;

	dots = 0;
	i = 0;
	while (s[i])
	{
		c = s[i];
		if (c == '.' && (i == 0 || s[i - 1] == '.' || s[i + 1] == '\0'))
			return (-1);
		if (c == '.')
			dots++;
		else if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')
				|| (c >= '0' && c <= '9') || c == '_' || (c == '$' && dollar)))
			return (-1);
		i++;
	}
	return (dots);
}

static int	class_full(t_apkmanifest *m)
{
	char	full[APK_CLASS_MAX];
	size_t	n;

	if (m->activity[0] == '\0')
		return (E_NOENT);
	full[0] = '\0';
	if (m->activity[0] == '.' || strchr(m->activity, '.') == NULL)
	{
		strlcpy(full, m->package, sizeof(full));
		if (m->activity[0] != '.')
			strlcat(full, ".", sizeof(full));
	}
	n = strlcat(full, m->activity, sizeof(full));
	if (n >= sizeof(full))
		return (E_RANGE);
	if (name_dots(full, 1) < 1)
		return (E_INVAL);
	memcpy(m->activity, full, n + 1);
	return (0);
}

static void	class_desc(t_apkmanifest *m)
{
	size_t	i;

	m->activity_desc[0] = 'L';
	i = 0;
	while (m->activity[i])
	{
		m->activity_desc[i + 1] = m->activity[i];
		if (m->activity[i] == '.')
			m->activity_desc[i + 1] = '/';
		i++;
	}
	m->activity_desc[i + 1] = ';';
	m->activity_desc[i + 2] = '\0';
}

static int	label_resolve(t_manwalk *w)
{
	t_arsc		arsc;
	t_resquery	q;
	uint8_t		*buf;
	int64_t		n;
	int			r;

	if (w->label_kind == RES_T_NULL)
		strlcpy(w->out->label, w->out->package, APK_LABEL_MAX);
	if (w->label_kind != RES_T_REFERENCE)
		return (0);
	n = apk_read(w->apk, "resources.arsc", &buf);
	if (n < 0)
		return ((int)n);
	q = (t_resquery){w->label_ref, "fr"};
	r = arsc_open(&arsc, (t_span){buf, (size_t)n});
	if (r == 0)
		r = arsc_string(&arsc, &q, (t_text){w->out->label, APK_LABEL_MAX});
	free(buf);
	if (r >= 0)
		r = apk_text_clean(w->out->label);
	return (r);
}

int	man_finish(t_manwalk *w)
{
	t_apkmanifest	*m;
	int				r;

	m = w->out;
	m->reason = APKR_PAQUET;
	if (strlen(m->package) > APK_PKG_MAX - 1 || name_dots(m->package, 0) < 1)
		return (E_INVAL);
	m->reason = APKR_ACTIVITE;
	r = class_full(m);
	if (r < 0)
		return (r);
	class_desc(m);
	m->reason = APKR_LIBELLE;
	r = label_resolve(w);
	if (r == E_NOMEM)
		m->reason = APKR_MEMOIRE;
	if (r < 0)
		return (r);
	m->reason = APKR_OK;
	return (0);
}
