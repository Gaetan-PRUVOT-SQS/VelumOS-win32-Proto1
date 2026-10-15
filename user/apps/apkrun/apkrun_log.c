#include "velum/libk.h"
#include "../common/apkglue.h"
#include "../common/platform.h"
#include "../common/tbuf.h"
#include "apkrun.h"

static void	emit(char *line)
{
	size_t	i;

	i = 0;
	while (line[i])
	{
		if ((unsigned char)line[i] < 0x20 || line[i] == 0x7f)
			line[i] = ' ';
		i++;
	}
	os_log(line);
}

void	apkrun_log_step(t_apkrun *app, const char *what, const char *detail)
{
	char	line[APKRUN_LINE_MAX];
	t_tbuf	b;

	tb_init(&b, line, sizeof(line));
	tb_str(&b, "apkrun: ");
	if (app->known)
		tb_str(&b, app->man.package);
	else if (app->path)
		tb_str(&b, app->path);
	tb_putc(&b, ' ');
	tb_str(&b, what);
	tb_str(&b, detail);
	emit(line);
}

int	apkrun_refuse(t_apkrun *app, const char *reason)
{
	apkglue_label(app->msg, sizeof(app->msg), reason);
	app->dead = 1;
	app->redraw = 1;
	apkrun_log_step(app, "refusé : ", app->msg);
	return (-1);
}

void	apkrun_applog(void *user, uint32_t level, const char *tag,
		const char *msg)
{
	char	line[APKRUN_LINE_MAX];
	t_tbuf	b;

	(void)user;
	(void)level;
	if (!tag || !msg)
		return ;
	tb_init(&b, line, sizeof(line));
	if (strncmp(tag, "apk", 3) == 0)
		tb_str(&b, "appli ");
	tb_str(&b, tag);
	tb_str(&b, ": ");
	tb_str(&b, msg);
	emit(line);
}

uint64_t	apkrun_clock(void *user)
{
	(void)user;
	return (os_wall_ns() / 1000000ull);
}
