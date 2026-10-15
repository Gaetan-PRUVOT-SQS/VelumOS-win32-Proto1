#include <stdio.h>
#include <string.h>
#include "d11.h"

int	d11_has(const char *hay, const char *needle)
{
	size_t	n;

	n = strlen(needle);
	while (*hay)
	{
		if (strncmp(hay, needle, n) == 0)
			return (1);
		hay++;
	}
	return (0);
}

static void	d11_note(t_d11 *t)
{
	const char	*name;

	name = dvm_class_name(dvm_class_of(t->vm, t->vm->pending));
	if (name)
		snprintf(t->exc, sizeof(t->exc), "%s", name);
	t->vm->pending = DVM_NULL;
}

int	d11_nat(t_d11 *t, const char *ref, const uint32_t *args)
{
	t_dname			nm;
	t_dclass		*c;
	const t_dmethod	*m;
	int				rc;

	t->ret = 0;
	t->exc[0] = '\0';
	d11_expand(t->txt, sizeof(t->txt), ref);
	if (sscanf(t->txt, "%127[^|]|%63[^|]|%127s", t->cls, t->name, t->sig) != 3)
		return (-1000);
	nm = (t_dname){t->name, t->sig};
	if (dvm_class(t->vm, t->cls, &c) != 0)
		return (-1001);
	if (dvm_method(t->vm, c, &nm, &m) != 0)
		return (-1002);
	rc = dvm_call(t->vm, m, args, &t->ret);
	if (rc == DVM_THROWN)
		d11_note(t);
	return (rc);
}

int64_t	d11_i2(t_d11 *t, const char *ref, uint32_t a, uint32_t b)
{
	uint32_t	args[5];
	int			rc;

	memset(args, 0, sizeof(args));
	args[0] = a;
	args[1] = b;
	rc = d11_nat(t, ref, args);
	if (rc != 0)
		return (-100000 + rc);
	return ((int32_t)t->ret);
}

int64_t	d11_calcul(t_d11 *t, const char *desc)
{
	char	ref[256];

	snprintf(ref, sizeof(ref), "%s|calcul|()I", desc);
	return (d11_i2(t, ref, 0, 0));
}
