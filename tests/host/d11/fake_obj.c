#include <stdio.h>
#include <string.h>
#include "d11.h"

t_dref	d11_str(t_d11 *t, const char *s)
{
	t_dref	r;

	r = DVM_NULL;
	if (dvm_string_utf8_new(t->vm, s, &r) != 0 || dvm_pin(t->vm, r) != 0)
		return (DVM_NULL);
	return (r);
}

const char	*d11_text(t_d11 *t, t_dref ref)
{
	t->txt[0] = '\0';
	if (dvm_string_utf8(t->vm, ref, (t_text){t->txt, sizeof(t->txt)}) < 0)
		return ("<pas une chaine>");
	return (t->txt);
}

t_dref	d11_new(t_d11 *t, const char *desc)
{
	t_dclass	*c;
	t_dref		r;

	r = DVM_NULL;
	if (dvm_class(t->vm, desc, &c) != 0 || dvm_new(t->vm, c, &r) != 0
		|| dvm_pin(t->vm, r) != 0)
		return (DVM_NULL);
	return (r);
}

t_dref	d11_view(t_d11 *t, const char *desc)
{
	uint32_t	args[2];
	char		ref[200];

	args[0] = d11_new(t, desc);
	args[1] = DVM_NULL;
	snprintf(ref, sizeof(ref), "%s|<init>|(Landroid/content/Context;)V", desc);
	if (d11_nat(t, ref, args) != 0)
		return (DVM_NULL);
	return (args[0]);
}

const char	*d11_long(char *buf, size_t n, const char *fin)
{
	memset(buf, 'a', n);
	memcpy(buf + n, fin, strlen(fin) + 1);
	return (buf);
}
