#include "dr_int.h"

static size_t	dr_cat(char *buf, size_t n, const char *s)
{
	size_t	i;

	i = 0;
	while (s && s[i] && n + 1 < DROID_TEXT_MAX)
		buf[n++] = s[i++];
	buf[n] = '\0';
	return (n);
}

static void	dr_describe(t_droid *d, t_dref exc)
{
	t_dstr16	s;
	t_dref		msg;
	size_t		n;

	n = dr_dotted(dvm_class_name(dvm_class_of(d->vm, exc)), d->error, 128);
	msg = DVM_NULL;
	if (dvm_throwable_message(d->vm, exc, &msg) != 0 || msg == DVM_NULL
		|| dvm_string_get(d->vm, msg, &s) != 0)
		return ;
	n = dr_cat(d->error, n, " : ");
	dr_utf8(s, d->error + n, DROID_TEXT_MAX - n);
}

int	dr_fail(t_droid *d, int rc)
{
	if (rc == 0)
		return (0);
	d->error[0] = '\0';
	if (rc == DVM_THROWN)
	{
		dr_describe(d, d->vm->pending);
		d->vm->pending = DVM_NULL;
		return (rc);
	}
	dr_cat(d->error, 0, "arrêt de la machine virtuelle");
	if (rc == E_TIMEOUT)
		dr_cat(d->error, 0, "budget d'instructions épuisé");
	if (rc == E_NOMEM)
		dr_cat(d->error, 0, "mémoire épuisée");
	return (rc);
}

int	dr_missing(t_droid *d, const t_dclass *c, const t_dname *nm)
{
	char	buf[DROID_TEXT_MAX];
	size_t	n;

	n = dr_cat(buf, 0, dvm_class_name(c));
	n = dr_cat(buf, n, "->");
	n = dr_cat(buf, n, nm->name);
	dr_cat(buf, n, nm->sig);
	return (dr_fail(d, dvm_throw(d->vm, DR_NSME, buf)));
}
