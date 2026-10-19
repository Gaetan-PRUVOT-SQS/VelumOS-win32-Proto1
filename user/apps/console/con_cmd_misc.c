#include "velum/err.h"
#include "velum/libk.h"
#include "console.h"

int	con_cmd_echo(t_console *c, int argc, char **argv)
{
	int	i;

	i = 1;
	while (i < argc)
	{
		if (i > 1)
			scr_putc(&c->screen, ' ', CC_TEXT);
		scr_puts(&c->screen, argv[i], CC_TEXT);
		i++;
	}
	scr_newline(&c->screen);
	return (0);
}

int	con_cmd_clear(t_console *c, int argc, char **argv)
{
	(void)argc;
	(void)argv;
	scr_clear(&c->screen);
	return (CON_QUIET);
}

int	con_cmd_exit(t_console *c, int argc, char **argv)
{
	(void)argc;
	(void)argv;
	c->quit = 1;
	return (CON_QUIET);
}

int	con_cmd_help(t_console *c, int argc, char **argv)
{
	const t_concmdent	*t;
	size_t				n;
	size_t				i;

	(void)argc;
	(void)argv;
	t = con_table(&n);
	i = 0;
	while (i < n)
	{
		scr_puts(&c->screen, t[i].name, CC_EXEC);
		scr_pad(&c->screen, (uint32_t)strlen(t[i].name), 8);
		con_say(c, t[i].help, CC_TEXT);
		i++;
	}
	con_say(c, "un nom de programme de /system/bin se lance directement",
		CC_DIM);
	return (0);
}

static int	resolve_program(t_console *c, const char *name, char *path)
{
	int	rc;

	if (strchr(name, '/'))
		rc = con_path_join(c->cwd, name, path, VFS_PATH_MAX);
	else
		rc = con_path_join(CON_BIN_DIR, name, path, VFS_PATH_MAX);
	if (rc < 0)
		return (rc);
	if (!con_path_in_bin(path))
		return (E_ACCES);
	return (0);
}

static int	spawn_arg(t_console *c, int argc, char **argv, char *norm,
				const char **out)
{
	char	extra[VFS_PATH_MAX];
	int		i;
	int		rc;

	*out = NULL;
	i = 1;
	while (i < argc)
	{
		rc = con_confine_arg(c->cwd, argv[i], i == 1 ? norm : extra,
				VFS_PATH_MAX);
		if (rc < 0)
			return (rc);
		if (i == 1)
			*out = norm;
		i++;
	}
	return (0);
}

static void	say_started(t_console *c, const char *name, uint32_t pid)
{
	char	num[24];

	con_fmt_num(num, sizeof(num), pid, 0);
	scr_puts(&c->screen, name, CC_EXEC);
	scr_puts(&c->screen, " lancé (pid ", CC_TEXT);
	scr_puts(&c->screen, num, CC_TEXT);
	con_say(c, ")", CC_TEXT);
}

int	con_run_program(t_console *c, int argc, char **argv)
{
	char		path[VFS_PATH_MAX];
	char		norm[VFS_PATH_MAX];
	t_vstat		st;
	const char	*arg;
	uint32_t	pid;
	int			rc;

	rc = resolve_program(c, argv[0], path);
	if (rc < 0)
		return (con_fail(c, argv[0], "commande introuvable"), 0);
	rc = c->ops->stat(c->env, path, &st);
	if (rc < 0 || (st.mode & S_TYPE_MASK) != S_TYPE_REG)
		return (con_fail(c, argv[0], "commande introuvable"), 0);
	rc = spawn_arg(c, argc, argv, norm, &arg);
	if (rc < 0)
		return (con_fail(c, argv[0], con_err_text(rc)), 0);
	pid = 0;
	rc = c->ops->spawn(c->env, path, arg, &pid);
	if (rc < 0)
		return (con_fail(c, argv[0], con_err_text(rc)), 0);
	say_started(c, argv[0], pid);
	return (0);
}
