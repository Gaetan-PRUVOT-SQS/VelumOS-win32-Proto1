#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "d11.h"

static void	d11_log(void *user, uint32_t level, const char *tag,
		const char *msg)
{
	t_d11	*t;

	t = user;
	t->nlogs++;
	t->level = level;
	snprintf(t->log, sizeof(t->log), "%s: %s", tag, msg);
}

static uint64_t	d11_clock(void *user)
{
	return (((t_d11 *)user)->now);
}

static t_span	d11_load(const char *name)
{
	char	path[512];
	FILE	*f;
	t_span	s;
	uint8_t	*buf;

	s = (t_span){NULL, 0};
	snprintf(path, sizeof(path), "%s/%s", D11_FIX, name);
	f = fopen(path, "rb");
	buf = malloc(1 << 20);
	if (f && buf)
		s = (t_span){buf, fread(buf, 1, 1 << 20, f)};
	else
		free(buf);
	if (f)
		fclose(f);
	return (s);
}

int	d11_open(t_d11 *t, const char *dex, uint32_t heap)
{
	t_dlimits	lim;
	int			rc;

	memset(t, 0, sizeof(*t));
	memset(&lim, 0, sizeof(lim));
	lim.heap_bytes = heap;
	t->now = 1234567890123ull;
	t->file = d11_load(dex);
	if (!t->file.p)
		return (-1000);
	rc = dvm_create(&t->vm, &lim);
	if (rc != 0)
		return (rc);
	t->d.log = d11_log;
	t->d.clock = d11_clock;
	t->d.user = t;
	rc = droid_install(&t->d, t->vm);
	if (rc == 0)
		rc = dvm_load_dex(t->vm, t->file);
	return (rc);
}

void	d11_close(t_d11 *t)
{
	dvm_destroy(t->vm);
	free((void *)(uintptr_t)t->file.p);
}
