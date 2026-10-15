#include <stdio.h>
#include "velum/err.h"
#include "velum/libk.h"
#include "platform.h"
#include "render.h"

void	os_log(const char *msg)
{
	strlcpy(g_fos.last_log, msg, sizeof(g_fos.last_log));
}

int	os_read_file(const char *path, char *buf, size_t max, size_t *len)
{
	FILE	*f;

	(void)path;
	f = fopen(g_fos.users_file, "rb");
	if (!f)
		return (E_NOENT);
	*len = fread(buf, 1, max, f);
	fclose(f);
	return (0);
}

void	render_reset(const char *users_file)
{
	memset(&g_fos, 0, sizeof(g_fos));
	memset(&g_fwm, 0, sizeof(g_fwm));
	g_fos.now = FOS_T0;
	g_fos.power_op = -1;
	g_fos.users_file = users_file;
	g_fwm.next_id = 1;
}
