#include <stdio.h>
#include <stdlib.h>
#include "harness.h"
#include "d07.h"

t_span	d07_load(const char *name)
{
	char	path[512];
	FILE	*f;
	t_span	s;
	uint8_t	*buf;

	s = (t_span){NULL, 0};
	snprintf(path, sizeof(path), "%s/%s", D07_FIX, name);
	f = fopen(path, "rb");
	buf = malloc(D07_LOAD_MAX);
	if (f && buf)
		s = (t_span){buf, fread(buf, 1, D07_LOAD_MAX, f)};
	else
		free(buf);
	if (f)
		fclose(f);
	return (s);
}

void	d07_free(t_span s)
{
	free((void *)(uintptr_t)s.p);
}

int	d07_verdict(t_span f, int *reason)
{
	t_apk		a;
	t_apksig	s;
	int			r;

	r = apk_open(&a, f);
	*reason = a.reason;
	if (r < 0)
		return (r);
	r = apk_verify(&a, &s);
	*reason = s.reason;
	return (r);
}

int	d07_file_verdict(const char *name, int *reason)
{
	t_span	f;
	int		r;

	*reason = -1;
	f = d07_load(name);
	if (f.p == NULL)
		return (D07_ABSENT);
	r = d07_verdict(f, reason);
	d07_free(f);
	return (r);
}

void	d07_table(const t_d07case *c, uint32_t n)
{
	uint32_t	i;
	int			reason;
	int			r;

	i = 0;
	while (i < n)
	{
		r = d07_file_verdict(c[i].file, &reason);
		if (c[i].want == D07_ANY)
			h_true(r < 0 && r != D07_ABSENT, c[i].file);
		else
		{
			h_eq_i64(c[i].file, r, c[i].want);
			h_eq_i64(c[i].file, reason, c[i].reason);
		}
		i++;
	}
}
