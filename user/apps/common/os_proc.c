#include "velum/velum.h"
#include "platform.h"

int	os_spawn(const char *path, const char *arg, uint32_t flags, t_handle *proc)
{
	const char	*argv[3];
	int64_t		h;

	argv[0] = path;
	argv[1] = arg;
	argv[2] = NULL;
	h = v_spawnv(path, argv, flags);
	if (h < 0)
		return ((int)h);
	*proc = (t_handle)h;
	return (0);
}

int	os_exited(t_handle proc)
{
	return (v_wait(proc, 0) == 0);
}

int	os_wait_exit(t_handle proc)
{
	return (v_wait(proc, TIMEOUT_INF));
}

void	os_kill(t_handle proc)
{
	v_proc_kill(proc, 0);
}

void	os_close(t_handle h)
{
	v_close(h);
}
