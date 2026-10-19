#include "velum/err.h"
#include "velum/libk.h"
#include "../common/tbuf.h"
#include "console.h"

void	con_init(t_console *c, const t_conops *ops, void *env, uint32_t cols)
{
	memset(c, 0, sizeof(*c));
	scr_init(&c->screen, cols);
	line_init(&c->line);
	strlcpy(c->cwd, "/", sizeof(c->cwd));
	c->sep = ">";
	c->ops = ops;
	c->env = env;
}

int	con_prompt_segs(const t_console *c, t_conseg *out)
{
	out[0] = (t_conseg){CON_USER, CC_USER};
	out[1] = (t_conseg){"@" CON_HOST, CC_AT};
	out[2] = (t_conseg){" ", CC_TEXT};
	out[3] = (t_conseg){c->cwd, CC_PATH};
	out[4] = (t_conseg){" ", CC_TEXT};
	out[5] = (t_conseg){c->sep, CC_TEXT};
	out[6] = (t_conseg){" ", CC_TEXT};
	return (CON_SEGS);
}

void	con_echo(t_console *c, const char *typed)
{
	t_conseg	seg[CON_SEGS];
	int			n;
	int			i;

	n = con_prompt_segs(c, seg);
	i = 0;
	while (i < n)
	{
		scr_puts(&c->screen, seg[i].text, seg[i].color);
		i++;
	}
	scr_puts(&c->screen, typed, CC_TEXT);
	scr_newline(&c->screen);
}

void	con_say(t_console *c, const char *text, uint8_t color)
{
	scr_puts(&c->screen, text, color);
	scr_newline(&c->screen);
}

void	con_fail(t_console *c, const char *cmd, const char *why)
{
	scr_puts(&c->screen, cmd, CC_ERR);
	scr_puts(&c->screen, " : ", CC_ERR);
	con_say(c, why, CC_ERR);
}

void	con_fmt_num(char *out, size_t size, uint64_t v, uint32_t width)
{
	char	d[24];
	size_t	n;
	size_t	len;

	n = 0;
	while (v || n == 0)
	{
		d[n++] = (char)('0' + v % 10);
		v /= 10;
	}
	len = 0;
	while (len + n < width && len + 1 < size)
		out[len++] = ' ';
	while (n > 0 && len + 1 < size)
		out[len++] = d[--n];
	if (size)
		out[len] = '\0';
}

void	con_fmt_duration(char *out, size_t size, uint64_t ns)
{
	t_tbuf		b;
	uint64_t	s;

	s = ns / 1000000000ull;
	tb_init(&b, out, size);
	if (s >= 3600)
	{
		tb_num(&b, s / 3600, 0);
		tb_str(&b, " h ");
		tb_num(&b, s % 3600 / 60, 0);
		tb_str(&b, " min");
		return ;
	}
	if (s >= 60)
	{
		tb_num(&b, s / 60, 0);
		tb_str(&b, " min ");
		s %= 60;
	}
	tb_num(&b, s, 0);
	tb_str(&b, " s");
}

void	con_exec(t_console *c, char *line)
{
	char				*argv[CON_ARGS_MAX];
	int					argc;
	const t_concmdent	*e;
	int					rc;

	argc = con_split(line, argv, CON_ARGS_MAX);
	if (argc < 0)
	{
		con_fail(c, "console", argc == E_RANGE ? "trop d'arguments"
			: "guillemet non fermé");
		con_say(c, "", CC_TEXT);
		return ;
	}
	if (argc == 0)
		return ;
	e = con_find(argv[0]);
	if (e)
		rc = e->run(c, argc, argv);
	else
		rc = con_run_program(c, argc, argv);
	if (rc != CON_QUIET)
		con_say(c, "", CC_TEXT);
}

void	con_submit(t_console *c)
{
	char	typed[CON_LINE_MAX];
	char	work[CON_LINE_MAX];

	line_commit(&c->line, typed);
	con_echo(c, typed);
	memcpy(work, typed, sizeof(work));
	con_exec(c, work);
}
