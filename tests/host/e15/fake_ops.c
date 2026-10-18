#include <string.h>
#include "velum/err.h"
#include "e15.h"

typedef struct s_node
{
	const char	*path;
	int			dir;
	const char	*data;
	size_t		len;
}	t_node;

static const t_node	g_fs[] = {
{"/", 1, NULL, 0},
{"/system", 1, NULL, 0},
{"/system/bin", 1, NULL, 0},
{"/system/bin/hello", 0, "ELF", 3},
{"/system/etc", 1, NULL, 0},
{"/system/etc/os-release", 0, "NAME=VelumOS\n", 13},
{"/docs", 1, NULL, 0},
{"/docs/notes.txt", 0, "ligne un\nligne deux\n", 20},
{"/docs/Zoo", 1, NULL, 0},
{"/docs/accents.txt", 0, "\xc3\xa9t\xc3\xa9", 5},
{"/docs/bin.dat", 0, "a\xff" "b\tc\n", 6}
};

uint32_t	g_fake_spawned;
char		g_fake_spawn_path[VFS_PATH_MAX];
char		g_fake_spawn_arg[64];
uint32_t	g_fake_killed;

void	fake_reset(void)
{
	g_fake_spawned = 0;
	g_fake_spawn_path[0] = '\0';
	g_fake_spawn_arg[0] = '\0';
	g_fake_killed = 0;
}

static const t_node	*find(const char *path)
{
	size_t	i;

	i = 0;
	while (i < sizeof(g_fs) / sizeof(g_fs[0]))
	{
		if (strcmp(g_fs[i].path, path) == 0)
			return (&g_fs[i]);
		i++;
	}
	return (NULL);
}

static int	f_stat(void *env, const char *path, t_vstat *st)
{
	const t_node	*n;

	(void)env;
	n = find(path);
	if (!n)
		return (E_NOENT);
	memset(st, 0, sizeof(*st));
	st->mode = n->dir ? S_TYPE_DIR : S_TYPE_REG;
	st->size = n->len;
	return (0);
}

static int	is_child(const char *dir, const char *path)
{
	size_t	dl;

	dl = strlen(dir);
	if (strcmp(dir, "/") == 0)
		dl = 0;
	if (strncmp(path, dir, dl) != 0 || path[dl] != '/' || path[dl + 1] == 0)
		return (0);
	return (strchr(path + dl + 1, '/') == NULL);
}

static int	f_list(void *env, const char *path, t_conent *out, uint32_t max)
{
	size_t		i;
	uint32_t	n;

	(void)env;
	n = 0;
	i = 0;
	while (i < sizeof(g_fs) / sizeof(g_fs[0]) && n < max)
	{
		if (is_child(path, g_fs[i].path))
		{
			strcpy(out[n].name, strrchr(g_fs[i].path, '/') + 1);
			out[n++].dir = (uint32_t)g_fs[i].dir;
		}
		i++;
	}
	return ((int)n);
}

static int	f_read(void *env, const char *path, char *buf, size_t max,
				size_t *len)
{
	const t_node	*n;

	(void)env;
	n = find(path);
	if (!n)
		return (E_NOENT);
	if (n->dir)
		return (E_ISDIR);
	*len = n->len < max ? n->len : max;
	memcpy(buf, n->data, *len);
	return (0);
}

static int	f_sysinfo(void *env, t_sysinfo *si)
{
	(void)env;
	memset(si, 0, sizeof(*si));
	si->uptime_ns = 14000000000ull;
	si->nprocs = 3;
	si->mem_total = 128ull << 20;
	si->mem_free = 87ull << 20;
	si->heap_live = 3ull << 20;
	return (0);
}

static int	f_procs(void *env, t_procinfo *out, uint32_t max)
{
	(void)env;
	(void)max;
	memset(out, 0, 3 * sizeof(*out));
	out[0].pid = 1;
	strcpy(out[0].name, "init");
	out[0].mem_bytes = 64 << 10;
	out[1].pid = 7;
	out[1].ppid = 1;
	strcpy(out[1].name, "winsrv");
	out[1].mem_bytes = 2048 << 10;
	out[2].pid = 42;
	out[2].ppid = 7;
	out[2].state = 2;
	strcpy(out[2].name, "hello");
	return (3);
}

static int	f_spawn(void *env, const char *path, const char *arg,
				uint32_t *pid)
{
	(void)env;
	strcpy(g_fake_spawn_path, path);
	if (arg)
		strcpy(g_fake_spawn_arg, arg);
	g_fake_spawned++;
	*pid = 42;
	return (0);
}

static int	f_kill(void *env, uint32_t pid)
{
	(void)env;
	if (pid != 42)
		return (E_NOENT);
	g_fake_killed = pid;
	return (0);
}

const t_conops	g_fake_ops = {f_stat, f_list, f_read, f_sysinfo, f_procs,
	f_spawn, f_kill};

void	screen_text(const t_conscreen *s, char *out, size_t size)
{
	uint32_t	i;
	size_t		len;
	const t_conrow	*r;

	len = 0;
	i = 0;
	while (i < scr_count(s))
	{
		r = scr_row(s, i);
		if (len + r->bytes + 2 >= size)
			break ;
		if (i > 0)
			out[len++] = '\n';
		memcpy(out + len, r->text, r->bytes);
		len += r->bytes;
		i++;
	}
	out[len] = '\0';
}

void	run_line(t_console *c, const char *line)
{
	char	work[CON_LINE_MAX];

	strcpy(work, line);
	con_exec(c, work);
}
