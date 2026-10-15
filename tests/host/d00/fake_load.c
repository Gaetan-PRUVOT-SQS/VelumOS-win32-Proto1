#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "d00.h"

t_span	d00_load(const char *name)
{
	char	path[512];
	FILE	*f;
	t_span	s;
	uint8_t	*buf;

	s = (t_span){NULL, 0};
	snprintf(path, sizeof(path), "%s/%s", D00_FIX, name);
	f = fopen(path, "rb");
	buf = malloc(1 << 22);
	if (f && buf)
		s = (t_span){buf, fread(buf, 1, 1 << 22, f)};
	else
		free(buf);
	if (f)
		fclose(f);
	return (s);
}

void	d00_free(t_span s)
{
	free((void *)(uintptr_t)s.p);
}

t_span	d00_entry(const t_zip *z, const char *name)
{
	t_zipent	e;
	uint8_t		*buf;

	if (zip_find(z, name, &e) < 0)
		return ((t_span){NULL, 0});
	buf = malloc(e.usize + 1);
	if (!buf || zip_extract(z, &e, buf, e.usize) != (int64_t)e.usize)
	{
		free(buf);
		return ((t_span){NULL, 0});
	}
	return ((t_span){buf, e.usize});
}

int	d00_apk_open(t_d00apk *a, const char *name)
{
	memset(a, 0, sizeof(*a));
	a->file = d00_load(name);
	if (!a->file.p || zip_open(&a->zip, a->file) < 0)
		return (-1);
	a->manifest = d00_entry(&a->zip, "AndroidManifest.xml");
	a->arsc = d00_entry(&a->zip, "resources.arsc");
	a->dex = d00_entry(&a->zip, "classes.dex");
	if (!a->manifest.p || !a->arsc.p || !a->dex.p)
		return (-2);
	return (0);
}

void	d00_apk_close(t_d00apk *a)
{
	d00_free(a->manifest);
	d00_free(a->arsc);
	d00_free(a->dex);
	d00_free(a->file);
}
