#include <string.h>
#include "d11.h"

static const char	*d11_alias(char c)
{
	if (c == 'S')
		return ("Ljava/lang/String;");
	if (c == 'O')
		return ("Ljava/lang/Object;");
	if (c == 'B')
		return ("Ljava/lang/StringBuilder;");
	if (c == 'C')
		return ("Ljava/lang/CharSequence;");
	if (c == 'T')
		return ("Landroid/widget/TextView;");
	if (c == 'L')
		return ("Landroid/view/View$OnClickListener;");
	return ("");
}

void	d11_expand(char *dst, size_t cap, const char *src)
{
	const char	*a;
	size_t		n;

	n = 0;
	while (*src && n + 64 < cap)
	{
		if (*src == '$')
		{
			a = d11_alias(src[1]);
			memcpy(dst + n, a, strlen(a));
			n += strlen(a);
			src += 2;
		}
		else
			dst[n++] = *src++;
	}
	dst[n] = '\0';
}

int64_t	d11_parse(t_d11 *t, const char *s)
{
	return (d11_i2(t, M_PARSE, d11_str(t, s), 0));
}

int64_t	d11_copie(t_d11 *t, const uint32_t *a)
{
	int	rc;

	rc = d11_nat(t, M_ACOPY, a);
	if (rc != 0)
		return (-100000 + rc);
	return (0);
}

int32_t	*d11_entiers(t_d11 *t, t_dref *out, int32_t len)
{
	t_darrview	v;
	int32_t		*p;
	int32_t		i;

	if (dvm_array_new(t->vm, "[I", len, out) != 0 || dvm_pin(t->vm, *out) != 0
		|| dvm_array_view(t->vm, *out, &v) != 0)
		return (NULL);
	p = v.data;
	i = 0;
	while (i < len)
	{
		p[i] = i + 1;
		i++;
	}
	return (p);
}
