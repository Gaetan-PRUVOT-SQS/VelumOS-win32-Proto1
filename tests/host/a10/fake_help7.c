#include "th.h"
#include "../../../kernel/input/inp_sys.h"
#include "../../../drivers/input/ps2.h"

int64_t	th_sys_setup(uint32_t kind)
{
	fake_all_reset();
	inp_sys_register();
	fproc_set(PF_INPUT);
	return (fsys_call(SYS_INPUT_OPEN, kind, 0, 0));
}

void	th_ps2_reset(void)
{
	fake_all_reset();
	f8042_reset();
}

void	th_irq(void)
{
	ps2_irq_handler(&g_ps2);
}

int	th_pop_all(t_inpevent *ev, int max)
{
	int	n;

	n = 0;
	while (n < max && input_pop(&ev[n]) > 0)
		n++;
	return (n);
}

int	th_cmd_index(uint8_t cmd, int from)
{
	int	i;

	i = from;
	while (i < (int)g_f8042.ncmds && i < F8_LOG)
	{
		if (g_f8042.cmds[i] == cmd)
			return (i);
		i++;
	}
	return (-1);
}
