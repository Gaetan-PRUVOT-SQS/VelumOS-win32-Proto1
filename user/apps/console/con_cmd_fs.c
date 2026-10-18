#include "velum/err.h"
#include "velum/libk.h"
#include "../common/utf8.h"
#include "console.h"

static int	resolve(t_console *c, const char *cmd, const char *arg, char *out)
{
	int	rc;

	rc = con_path_join(c->cwd, arg, out, VFS_PATH_MAX);
	if (rc < 0)
		con_fail(c, cmd, con_err_text(rc));
	return (rc);
}

static int	stat_path(t_console *c, const char *cmd, const char *path,
				t_vstat *st)
{
	int	rc;

	rc = c->ops->stat(c->env, path, st);
	if (rc < 0)
		con_fail(c, cmd, con_err_text(rc));
	return (rc);
}

int	con_cmd_pwd(t_console *c, int argc, char **argv)
{
	(void)argc;
	(void)argv;
	con_say(c, c->cwd, CC_PATH);
	return (0);
}

int	con_cmd_cd(t_console *c, int argc, char **argv)
{
	char	path[VFS_PATH_MAX];
	t_vstat	st;

	if (argc > 2)
		return (con_fail(c, "cd", "trop d'arguments"), 0);
	if (resolve(c, "cd", argc == 2 ? argv[1] : "/", path) < 0)
		return (0);
	if (stat_path(c, "cd", path, &st) < 0)
		return (0);
	if ((st.mode & S_TYPE_MASK) != S_TYPE_DIR)
		return (con_fail(c, "cd", "n'est pas un dossier"), 0);
	strlcpy(c->cwd, path, sizeof(c->cwd));
	return (0);
}

static void	sort_ents(t_conent *e, uint32_t n)
{
	t_conent	tmp;
	uint32_t	i;
	uint32_t	j;

	i = 1;
	while (i < n)
	{
		tmp = e[i];
		j = i;
		while (j > 0 && (e[j - 1].dir < tmp.dir
				|| (e[j - 1].dir == tmp.dir
					&& strcmp(e[j - 1].name, tmp.name) > 0)))
		{
			e[j] = e[j - 1];
			j--;
		}
		e[j] = tmp;
		i++;
	}
}

static uint8_t	ent_color(const t_console *c, const t_conent *e,
					const char *dir)
{
	if (e->dir)
		return (CC_DIR);
	if (strcmp(dir, CON_BIN_DIR) == 0)
		return (CC_EXEC);
	(void)c;
	return (CC_TEXT);
}

static void	put_ent(t_console *c, const t_conent *e, const char *dir)
{
	uint32_t	w;
	const t_conrow	*last;

	w = line_cells(e->name, (uint32_t)strlen(e->name)) + (e->dir ? 1 : 0);
	last = scr_row(&c->screen, scr_count(&c->screen) - 1);
	if (last->cells > 0 && last->cells + w + 2 > c->screen.cols)
		scr_newline(&c->screen);
	else if (last->cells > 0)
		scr_pad(&c->screen, 0, 2);
	scr_puts(&c->screen, e->name, ent_color(c, e, dir));
	if (e->dir)
		scr_putc(&c->screen, '/', CC_DIR);
}

int	con_cmd_ls(t_console *c, int argc, char **argv)
{
	char	path[VFS_PATH_MAX];
	t_vstat	st;
	int		n;
	int		i;

	if (argc > 2)
		return (con_fail(c, "ls", "trop d'arguments"), 0);
	if (resolve(c, "ls", argc == 2 ? argv[1] : ".", path) < 0
		|| stat_path(c, "ls", path, &st) < 0)
		return (0);
	if ((st.mode & S_TYPE_MASK) != S_TYPE_DIR)
		return (con_say(c, argc == 2 ? argv[1] : path, CC_TEXT), 0);
	n = c->ops->list(c->env, path, c->ents, CON_LS_MAX);
	if (n < 0)
		return (con_fail(c, "ls", con_err_text(n)), 0);
	sort_ents(c->ents, (uint32_t)n);
	i = 0;
	while (i < n)
		put_ent(c, &c->ents[i++], path);
	if (n > 0)
		scr_newline(&c->screen);
	if (n == CON_LS_MAX)
		con_say(c, "liste tronquée", CC_DIM);
	return (0);
}

static size_t	put_glyph(t_console *c, const char *s, size_t left)
{
	size_t	k;
	size_t	i;

	k = utf8_seq_len((const uint8_t *)s, left);
	if (k == 0)
	{
		scr_putc(&c->screen, '?', CC_TEXT);
		return (1);
	}
	i = 0;
	while (i < k)
		scr_putc(&c->screen, s[i++], CC_TEXT);
	return (k);
}

static void	put_text(t_console *c, const char *s, size_t len)
{
	size_t	i;

	i = 0;
	while (i < len)
	{
		if (s[i] == '\n')
			scr_newline(&c->screen);
		else if (s[i] == '\t')
			scr_pad(&c->screen, 0, 4);
		if (s[i] == '\n' || s[i] == '\t' || s[i] == '\r'
			|| ((uint8_t)s[i] < 0x20) || s[i] == 0x7f)
			i++;
		else
			i += put_glyph(c, s + i, len - i);
	}
}

int	con_cmd_cat(t_console *c, int argc, char **argv)
{
	char	path[VFS_PATH_MAX];
	size_t	len;
	int		rc;

	if (argc != 2)
		return (con_fail(c, "cat", "usage : cat fichier"), 0);
	if (resolve(c, "cat", argv[1], path) < 0)
		return (0);
	len = 0;
	rc = c->ops->read(c->env, path, c->io, CON_CAT_MAX, &len);
	if (rc < 0)
		return (con_fail(c, "cat", con_err_text(rc)), 0);
	put_text(c, c->io, len);
	if (len > 0 && c->io[len - 1] != '\n')
		scr_newline(&c->screen);
	if (len == CON_CAT_MAX)
		con_say(c, "... affichage limité à 4096 octets", CC_DIM);
	return (0);
}
