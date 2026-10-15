#include <string.h>
#include "fake.h"

t_fakecmds	g_fake_cmds;

void	fake_cmd_reset(void)
{
	memset(&g_fake_cmds, 0, sizeof(g_fake_cmds));
}

void	fake_cmd(void *c, uint32_t code, void *user)
{
	t_fakecmd	*e;

	(void)user;
	if (g_fake_cmds.n >= FAKE_CMD_MAX)
		return ;
	e = &g_fake_cmds.list[g_fake_cmds.n];
	g_fake_cmds.n++;
	e->id = ((t_ctl *)c)->id;
	e->code = code;
	e->ctl = c;
}

void	fake_cb(void *c, uint32_t code, void *user)
{
	g_fake_cmds.cbs++;
	fake_cmd(c, code | FAKE_CB_MARK, user);
}

int	fake_cmd_count(void)
{
	return (g_fake_cmds.n);
}

int	fake_cmd_code(int i)
{
	if (i < 0 || i >= g_fake_cmds.n)
		return (-1);
	return ((int)g_fake_cmds.list[i].code);
}
