#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "fuzz.h"

#define REPLAY_MAX_BYTES 0x4000000u

typedef struct s_replay
{
	unsigned			files;
	unsigned			skipped;
	unsigned long long	bytes;
}	t_replay;

static int	replay_cmp(const void *a, const void *b)
{
	return (strcmp(*(char *const *)a, *(char *const *)b));
}

static int	replay_read(const char *path, uint8_t **buf, size_t *len)
{
	FILE		*f;
	struct stat	st;
	size_t		got;

	f = fopen(path, "rb");
	if (f == NULL)
		return (-1);
	if (fstat(fileno(f), &st) != 0 || st.st_size < 0
		|| (unsigned long long)st.st_size > REPLAY_MAX_BYTES)
	{
		fclose(f);
		return (-1);
	}
	*len = (size_t)st.st_size;
	*buf = malloc(*len + (*len == 0));
	if (*buf == NULL)
		abort();
	got = fread(*buf, 1, *len, f);
	fclose(f);
	if (got != *len)
	{
		free(*buf);
		return (-1);
	}
	return (0);
}

static void	replay_file(const char *path, t_replay *r)
{
	uint8_t	*buf;
	size_t	len;

	if (replay_read(path, &buf, &len) != 0)
	{
		fprintf(stderr, "rejeu : lecture impossible : %s\n", path);
		r->skipped++;
		return ;
	}
	LLVMFuzzerTestOneInput(buf, len);
	free(buf);
	r->files++;
	r->bytes += len;
}

static char	**replay_list(const char *dir, size_t *count)
{
	DIR				*d;
	struct dirent	*e;
	char			**v;
	size_t			n;
	size_t			m;

	d = opendir(dir);
	if (d == NULL)
		return (NULL);
	v = NULL;
	n = 0;
	while ((e = readdir(d)) != NULL)
	{
		if (e->d_name[0] == '.')
			continue ;
		v = realloc(v, (n + 1) * sizeof(*v));
		if (v == NULL)
			abort();
		m = strlen(dir) + strlen(e->d_name) + 2;
		v[n] = malloc(m);
		if (v[n] == NULL)
			abort();
		snprintf(v[n++], m, "%s/%s", dir, e->d_name);
	}
	closedir(d);
	qsort(v, n, sizeof(*v), replay_cmp);
	*count = n;
	return (v);
}

static void	replay_path(const char *path, t_replay *r)
{
	struct stat	st;
	char		**v;
	size_t		n;
	size_t		i;

	if (stat(path, &st) != 0)
	{
		fprintf(stderr, "rejeu : introuvable : %s\n", path);
		r->skipped++;
		return ;
	}
	if (!S_ISDIR(st.st_mode))
	{
		replay_file(path, r);
		return ;
	}
	n = 0;
	v = replay_list(path, &n);
	i = 0;
	while (i < n)
	{
		replay_path(v[i], r);
		free(v[i++]);
	}
	free(v);
}

int	main(int argc, char **argv)
{
	t_replay	r;
	int			i;

	memset(&r, 0, sizeof(r));
	i = 1;
	while (i < argc)
		replay_path(argv[i++], &r);
	printf("rejeu : %u entrees, %llu octets, %u illisibles\n", r.files,
		r.bytes, r.skipped);
	return (r.skipped != 0 || r.files == 0);
}
