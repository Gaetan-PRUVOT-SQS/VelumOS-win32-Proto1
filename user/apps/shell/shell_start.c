#include "../common/platform.h"
#include "../common/typo.h"
#include "shell.h"

int	main(int argc, char **argv)
{
	t_shell	sh;
	int		r;

	r = sh_boot(&sh, argc, argv);
	if (r < 0)
	{
		os_log("shell: démarrage impossible (serveur de fenêtres" NBSP "?)");
		return (1);
	}
	os_log("shell: bureau prêt");
	sh_run(&sh);
	sh_shutdown(&sh);
	os_log("shell: fin de session");
	return (0);
}
