#include "velum/libk.h"
#include "velum/abi/abi_syscall.h"
#include "../common/platform.h"
#include "shell.h"

void	sh_reap(t_shell *sh)
{
	uint32_t	i;

	i = 0;
	while (i < sh->nchildren)
	{
		if (os_exited(sh->children[i]))
		{
			os_close(sh->children[i]);
			sh->nchildren--;
			sh->children[i] = sh->children[sh->nchildren];
		}
		else
			i++;
	}
}

static uint32_t	sh_flags_for(const char *path)
{
	if (strcmp(path, "/system/bin/fichiers") == 0
		|| strcmp(path, "/system/bin/reglages") == 0)
		return (PF_FSWRITE);
	return (0);
}

int	sh_launch_arg(t_shell *sh, const char *path, const char *arg)
{
	t_handle	h;
	int			r;

	sh_reap(sh);
	if (arg && arg[0] == '\0')
		arg = NULL;
	r = os_spawn(path, arg, sh_flags_for(path), &h);
	if (r < 0)
		return (r);
	if (sh->nchildren >= SH_CHILD_MAX)
	{
		os_close(h);
		return (0);
	}
	sh->children[sh->nchildren] = h;
	sh->nchildren++;
	return (0);
}

int	sh_launch(t_shell *sh, const char *path)
{
	return (sh_launch_arg(sh, path, NULL));
}

void	sh_kill_children(t_shell *sh)
{
	uint32_t	i;

	i = 0;
	while (i < sh->nchildren)
	{
		os_kill(sh->children[i]);
		os_close(sh->children[i]);
		i++;
	}
	sh->nchildren = 0;
}
