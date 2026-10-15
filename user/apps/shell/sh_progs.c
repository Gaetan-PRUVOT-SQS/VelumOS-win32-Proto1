#include "velum/libk.h"
#include "../common/platform.h"
#include "shell.h"

void	sh_load_programs(t_shell *sh)
{
	static char	buf[SM_PROGS_FILE_MAX + 1];
	size_t		len;
	int			r;

	len = 0;
	r = os_read_file(SM_PROGS_PATH, buf, sizeof(buf), &len);
	if (r == 0)
	{
		r = sm_progs_parse(buf, len, &sh->progs);
		if (r < 0)
			os_log("shell: /system/etc/programmes invalide, liste intégrée");
	}
	if (r < 0)
		sm_progs_default(&sh->progs);
	sm_set_programs(&sh->progs);
}

void	sh_launch_program(t_shell *sh, uint32_t cmd)
{
	int32_t	i;

	i = sm_prog_of(cmd);
	if (i >= 0 && (uint32_t)i < sh->progs.count)
		sh_launch_arg(sh, sh->progs.list[i].path, sh->progs.list[i].arg);
}
