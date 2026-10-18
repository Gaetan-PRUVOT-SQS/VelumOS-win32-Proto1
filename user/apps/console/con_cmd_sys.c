#include "velum/err.h"
#include "velum/libk.h"
#include "../common/tbuf.h"
#include "console.h"

static const char	*state_name(uint32_t st)
{
	if (st == 0)
		return ("actif");
	if (st == 1)
		return ("sortie");
	return ("zombie");
}

static void	ps_line(t_console *c, const t_procinfo *p)
{
	char	num[24];
	char	name[sizeof(p->name) + 1];

	con_fmt_num(num, sizeof(num), p->pid, 5);
	scr_puts(&c->screen, num, CC_TEXT);
	con_fmt_num(num, sizeof(num), p->ppid, 6);
	scr_puts(&c->screen, num, CC_TEXT);
	scr_puts(&c->screen, "  ", CC_TEXT);
	scr_puts(&c->screen, state_name(p->state), CC_TEXT);
	scr_pad(&c->screen, 0, 8 - (uint32_t)strlen(state_name(p->state)));
	con_fmt_num(num, sizeof(num), p->mem_bytes >> 10, 8);
	scr_puts(&c->screen, num, CC_TEXT);
	scr_puts(&c->screen, " Kio  ", CC_TEXT);
	memcpy(name, p->name, sizeof(p->name));
	name[sizeof(p->name)] = '\0';
	scr_puts(&c->screen, name, CC_EXEC);
	scr_newline(&c->screen);
}

int	con_cmd_ps(t_console *c, int argc, char **argv)
{
	int	n;
	int	i;

	(void)argc;
	(void)argv;
	n = c->ops->procs(c->env, c->procs, CON_PS_MAX);
	if (n < 0)
		return (con_fail(c, "ps", con_err_text(n)), 0);
	con_say(c, "  PID  PPID  ETAT        MEMOIRE  NOM", CC_DIM);
	i = 0;
	while (i < n)
		ps_line(c, &c->procs[i++]);
	return (0);
}

static int	parse_pid(const char *s, uint32_t *pid)
{
	uint64_t	v;

	if (!*s)
		return (E_INVAL);
	v = 0;
	while (*s)
	{
		if (*s < '0' || *s > '9')
			return (E_INVAL);
		v = v * 10 + (uint64_t)(*s - '0');
		if (v > 0xffffffffull)
			return (E_RANGE);
		s++;
	}
	*pid = (uint32_t)v;
	return (0);
}

int	con_cmd_kill(t_console *c, int argc, char **argv)
{
	uint32_t	pid;
	int			rc;

	if (argc != 2)
		return (con_fail(c, "kill", "usage : kill pid"), 0);
	if (parse_pid(argv[1], &pid) < 0)
		return (con_fail(c, "kill", "pid invalide"), 0);
	rc = c->ops->kill(c->env, pid);
	if (rc == E_NOENT)
		return (con_fail(c, "kill", "processus inconnu de la console"), 0);
	if (rc < 0)
		return (con_fail(c, "kill", con_err_text(rc)), 0);
	con_say(c, "signal envoyé", CC_TEXT);
	return (0);
}

static void	say_mem(t_console *c, const t_sysinfo *si, char *out, size_t size)
{
	t_tbuf	b;

	tb_init(&b, out, size);
	tb_str(&b, "mémoire ");
	tb_num(&b, (si->mem_total - si->mem_free) >> 20, 0);
	tb_str(&b, " / ");
	tb_num(&b, si->mem_total >> 20, 0);
	tb_str(&b, " Mo");
	(void)c;
}

int	con_cmd_uptime(t_console *c, int argc, char **argv)
{
	t_sysinfo	si;
	char		up[32];
	char		mem[48];
	char		line[160];
	t_tbuf		b;
	int			rc;

	(void)argc;
	(void)argv;
	rc = c->ops->sysinfo(c->env, &si);
	if (rc < 0)
		return (con_fail(c, "uptime", con_err_text(rc)), 0);
	con_fmt_duration(up, sizeof(up), si.uptime_ns);
	say_mem(c, &si, mem, sizeof(mem));
	tb_init(&b, line, sizeof(line));
	tb_str(&b, "en marche depuis ");
	tb_str(&b, up);
	tb_str(&b, " · ");
	tb_num(&b, si.nprocs, 0);
	tb_str(&b, " processus · ");
	tb_str(&b, mem);
	con_say(c, line, CC_TEXT);
	return (0);
}

static void	mem_row(t_console *c, const char *label, uint64_t bytes)
{
	char	num[24];

	scr_puts(&c->screen, label, CC_DIM);
	scr_pad(&c->screen, (uint32_t)strlen(label), 14);
	con_fmt_num(num, sizeof(num), bytes >> 10, 9);
	scr_puts(&c->screen, num, CC_TEXT);
	con_say(c, " Kio", CC_TEXT);
}

int	con_cmd_mem(t_console *c, int argc, char **argv)
{
	t_sysinfo	si;
	int			rc;

	(void)argc;
	(void)argv;
	rc = c->ops->sysinfo(c->env, &si);
	if (rc < 0)
		return (con_fail(c, "mem", con_err_text(rc)), 0);
	mem_row(c, "total", si.mem_total);
	mem_row(c, "libre", si.mem_free);
	mem_row(c, "utilisée", si.mem_total - si.mem_free);
	mem_row(c, "tas noyau", si.heap_live);
	return (0);
}
