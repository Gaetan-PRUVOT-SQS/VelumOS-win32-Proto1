#include "velum/err.h"
#include "velum/libk.h"
#include "velum/velum.h"
#include "../common/platform.h"
#include "con_app.h"

#define LIST_BATCH 8

static int	sys_stat(void *env, const char *path, t_vstat *st)
{
	(void)env;
	return (v_stat(path, st));
}

static void	copy_name(char *dst, const char *src)
{
	size_t	n;

	n = strlcpy(dst, src, CON_NAME_MAX);
	if (n < CON_NAME_MAX)
		return ;
	n = CON_NAME_MAX - 1;
	while (n > 0 && ((uint8_t)dst[n] & 0xc0) == 0x80)
		n--;
	dst[n] = '\0';
}

static int	keep_entry(const t_dirent *d)
{
	return (strcmp(d->name, ".") != 0 && strcmp(d->name, "..") != 0);
}

static int	sys_list(void *env, const char *path, t_conent *out, uint32_t max)
{
	t_dirent	batch[LIST_BATCH];
	int64_t		h;
	uint32_t	n;
	int			got;
	int			i;

	(void)env;
	h = v_open(path, O_RDONLY | O_DIRECTORY, 0);
	if (h < 0)
		return ((int)h);
	n = 0;
	got = v_readdir((t_handle)h, batch, LIST_BATCH);
	while (got > 0 && n < max)
	{
		i = 0;
		while (i < got && n < max)
		{
			if (keep_entry(&batch[i]))
			{
				copy_name(out[n].name, batch[i].name);
				out[n++].dir = batch[i].type == S_TYPE_DIR;
			}
			i++;
		}
		got = v_readdir((t_handle)h, batch, LIST_BATCH);
	}
	v_close((t_handle)h);
	if (got < 0 && n == 0)
		return (got);
	return ((int)n);
}

static int	sys_read(void *env, const char *path, char *buf, size_t max,
				size_t *len)
{
	(void)env;
	return (os_read_file(path, buf, max, len));
}

static int	sys_sysinfo(void *env, t_sysinfo *si)
{
	(void)env;
	return (os_sysinfo(si));
}

static int	sys_procs(void *env, t_procinfo *out, uint32_t max)
{
	(void)env;
	return (v_proc_list(out, max));
}

void	con_jobs_reap(t_conjobs *j)
{
	uint32_t	i;

	i = 0;
	while (i < j->count)
	{
		if (os_exited(j->handle[i]))
		{
			os_close(j->handle[i]);
			j->count--;
			j->handle[i] = j->handle[j->count];
			j->pid[i] = j->pid[j->count];
		}
		else
			i++;
	}
}

static int	sys_spawn(void *env, const char *path, const char *arg,
				uint32_t *pid)
{
	t_conjobs	*j;
	t_handle	h;
	t_procinfo	pi;
	int			rc;

	j = &((t_conapp *)env)->jobs;
	con_jobs_reap(j);
	if (j->count >= CON_JOBS_MAX)
		return (E_BUSY);
	rc = os_spawn(path, arg, 0, &h);
	if (rc < 0)
		return (rc);
	*pid = 0;
	if (v_proc_info(h, &pi) == 0)
		*pid = pi.pid;
	j->handle[j->count] = h;
	j->pid[j->count] = *pid;
	j->count++;
	return (0);
}

static int	sys_kill(void *env, uint32_t pid)
{
	t_conjobs	*j;
	uint32_t	i;

	j = &((t_conapp *)env)->jobs;
	con_jobs_reap(j);
	i = 0;
	while (i < j->count)
	{
		if (j->pid[i] == pid)
		{
			os_kill(j->handle[i]);
			return (0);
		}
		i++;
	}
	return (E_NOENT);
}

const t_conops	g_conops_system = {sys_stat, sys_list, sys_read, sys_sysinfo,
	sys_procs, sys_spawn, sys_kill};
