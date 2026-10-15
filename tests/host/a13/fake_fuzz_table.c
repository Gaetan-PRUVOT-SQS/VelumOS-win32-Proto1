#include <stdio.h>
#include <string.h>
#include <time.h>
#include "display_int.h"
#include "fuzz_table.h"
#include "velum/err.h"
#include "velum/uptr.h"
#include "velum/vmm.h"

static const int64_t	g_documented[] = {E_PERM, E_NOENT, E_IO, E_BADF,
	E_CHILD, E_AGAIN, E_NOMEM, E_ACCES, E_FAULT, E_BUSY, E_EXIST, E_NODEV,
	E_NOTDIR, E_ISDIR, E_INVAL, E_NFILE, E_MFILE, E_NOSPC, E_PIPE, E_RANGE,
	E_DEADLK, E_NOSYS, E_NOTEMPTY, E_PROTO, E_OVERFLOW, E_NOTSUP, E_TIMEOUT,
	E_CANCELED, UPTR_E2BIG};

static const uint64_t	g_edges[] = {0, 1, UINT64_MAX, UINT64_MAX - 1,
	0x8000000000000000ull, 0x7fffffffffffffffull, 0xffffffffull,
	0x100000000ull, 0x80000000ull, 0xffff800000000000ull,
	0xffffffff80000000ull, USER_TOP, USER_TOP + 1, USER_TOP - 1,
	USER_MIN, USER_MIN - 1, 4095, 4096, 4097, FAKE_BAD_PTR, UPTR_COPY_MAX,
	UPTR_COPY_MAX + 1, 0x40000000ull, 0x7fffffffffffull};

bool	fz_documented(int64_t rc)
{
	size_t	i;

	if (rc >= 0)
		return (true);
	i = 0;
	while (i < sizeof(g_documented) / sizeof(g_documented[0]))
	{
		if (g_documented[i] == rc)
			return (true);
		i++;
	}
	return (false);
}

uint32_t	fz_scan(uint32_t *nums)
{
	uint32_t	n;
	uint32_t	i;

	n = 0;
	i = 0;
	while (i < FZ_SLOTS)
	{
		if (g_fp.sys[i])
			nums[n++] = i;
		i++;
	}
	return (n);
}

bool	fz_registered(uint32_t num)
{
	return (num < FZ_SLOTS && g_fp.sys[num] != NULL);
}

static uint64_t	pick_arg(t_fzrun *r)
{
	uint64_t	v;
	uint64_t	k;

	v = kfix_rnd(&r->seed);
	k = v >> 8;
	if ((v & 7) < 3)
		return (g_edges[k % (sizeof(g_edges) / sizeof(g_edges[0]))]);
	if ((v & 7) == 3)
		return ((uint64_t)(uintptr_t)r->buf + k % FZ_BUF);
	if ((v & 7) == 4)
		return ((uint64_t)(uintptr_t)&g_fp);
	if ((v & 7) == 5)
		return (k % 4096);
	if ((v & 7) == 6)
		return (k << (k % 40));
	return (k);
}

static uint32_t	pick_num(t_fzrun *r)
{
	uint64_t	v;

	v = kfix_rnd(&r->seed);
	if ((v & 15) == 0)
		return ((uint32_t)(v >> 8) | 0x100u);
	if ((v & 1) && r->registered_count)
		return (r->registered[(v >> 8) % r->registered_count]);
	return ((uint32_t)((v >> 8) % FZ_SLOTS));
}

void	fz_gen(t_fzrun *r, t_fzcall *c)
{
	uint64_t	v;
	int			i;

	c->num = pick_num(r);
	i = 0;
	while (i < 6)
	{
		c->a[i] = pick_arg(r);
		i++;
	}
	v = kfix_rnd(&r->seed);
	c->pid = 10 + (uint32_t)(v % 3);
	c->flags = (uint32_t)((v >> 8) % 128);
	c->no_proc = (v >> 16) % 50 == 0;
	g_fp.ok_base = (uint64_t)(uintptr_t)r->buf;
	g_fp.ok_len = FZ_WRITABLE;
}

int64_t	fz_call(const t_fzcall *c, bool *halted)
{
	t_sysargs	args;
	int64_t		rc;

	memset(&args, 0, sizeof(args));
	memcpy(args.a, c->a, sizeof(args.a));
	fake_proc_set(c->pid, c->flags);
	if (c->no_proc)
		g_fp.cur = NULL;
	*halted = false;
	if (setjmp(g_fk.halt_env))
	{
		*halted = true;
		return (E_IO);
	}
	if (c->num >= FZ_SLOTS || !g_fp.sys[c->num])
		return (E_NOSYS);
	rc = g_fp.sys[c->num](&args);
	return (rc);
}

static bool	is_display(uint32_t num)
{
	return (num >= SYS_DISPLAY_INFO && num <= SYS_KCON);
}

static bool	privileged(const t_fzcall *c)
{
	return (!c->no_proc && (c->flags & PF_DISPLAY));
}

static bool	any_above_u32(const t_fzcall *c, int count)
{
	int	i;

	i = 0;
	while (i < count)
	{
		if (c->a[i] > UINT32_MAX)
			return (true);
		i++;
	}
	return (false);
}

static const char	*contract_gated(const t_fzcall *c, int64_t rc)
{
	if (!privileged(c))
		return (rc == E_PERM ? NULL : "appel prive sans droit : E_PERM attendu");
	if (rc == E_PERM)
		return ("E_PERM alors que le droit d'affichage est present");
	if (c->num == SYS_DISPLAY_SET_MODE && any_above_u32(c, 3))
		return (rc == E_INVAL ? NULL : "argument au-dela de 32 bits : "
			"E_INVAL attendu");
	if (c->num == SYS_KCON && c->a[0] > 1)
		return (rc == E_INVAL ? NULL : "KCON hors 0 ou 1 : E_INVAL attendu");
	return (NULL);
}

static const char	*contract(const t_fzcall *c, int64_t rc)
{
	if (c->num == SYS_DISPLAY_MAP || c->num == SYS_DISPLAY_SET_MODE
		|| c->num == SYS_KCON)
		return (contract_gated(c, rc));
	if (rc == E_PERM)
		return ("E_PERM sur un appel sans droit requis");
	if (c->num == SYS_DISPLAY_MODES && c->a[1] > UINT32_MAX)
		return (rc == E_INVAL ? NULL : "nombre de modes > 32 bits : "
			"E_INVAL attendu");
	if (c->num == SYS_DISPLAY_MODES && rc > DISP_MODES_MAX)
		return ("plus de modes rendus que DISP_MODES_MAX");
	if (c->num == SYS_DISPLAY_INFO && rc > 0)
		return ("DISPLAY_INFO rend 0 ou une erreur");
	return (NULL);
}

const char	*fz_verdict(const t_fzrun *r, const t_fzcall *c, int64_t rc)
{
	const char	*why;

	if (!fz_registered(c->num) && rc != E_NOSYS)
		return ("numero absent de la table sans E_NOSYS");
	if (fz_registered(c->num) && rc == E_NOSYS)
		return ("appel enregistre qui rend E_NOSYS");
	if (!fz_documented(rc))
		return ("code d'erreur hors liste documentee");
	if (is_display(c->num) && !fx_rc_ok(rc))
		return ("code d'erreur hors liste de l'appel d'affichage");
	why = fz_registered(c->num) ? contract(c, rc) : NULL;
	if (why)
		return (why);
	if (c->num == SYS_DISPLAY_MAP && rc >= 0 && rc % 4096)
		return ("adresse de mappage non alignee");
	if (c->num == SYS_DISPLAY_MAP && rc >= 0 && (uint64_t)rc < USER_MIN)
		return ("adresse de mappage sous USER_MIN");
	if (g_fp.locks != g_fp.unlocks)
		return ("verrou non rendu");
	if (g_fp.bad_locks)
		return ("erreur de verrou");
	if (!fake_vmm_guards_ok())
		return ("ecriture hors des mappages");
	if (fake_vmm_live_windows() != 1)
		return ("fenetre noyau perdue ou dupliquee");
	if (fake_vmm_live_maps() > FZ_MAPS_MAX)
		return ("mappages vivants en trop");
	if (fx_canary(r->buf + FZ_WRITABLE, FZ_BUF - FZ_WRITABLE) != 0)
		return ("ecriture dans la zone interdite");
	return (NULL);
}

void	fz_report(const t_fzrun *r, const t_fzcall *c, int64_t rc,
			const char *invariant)
{
	printf("INCIDENT graine=%llu appel=%llu num=%#x pid=%u flags=%#x "
		"sans_proc=%d a=[%#llx %#llx %#llx %#llx %#llx %#llx] rc=%lld : %s\n",
		(unsigned long long)r->seed_start,
		(unsigned long long)r->calls, c->num, c->pid, c->flags, c->no_proc,
		(unsigned long long)c->a[0], (unsigned long long)c->a[1],
		(unsigned long long)c->a[2], (unsigned long long)c->a[3],
		(unsigned long long)c->a[4], (unsigned long long)c->a[5],
		(long long)rc, invariant);
}

void	fz_open(t_fzrun *r, uint64_t seed, uint8_t *buf)
{
	memset(r, 0, sizeof(*r));
	r->seed_start = seed;
	r->seed = seed ? seed : 1;
	r->buf = buf;
	memset(buf, 0xaa, FZ_BUF);
	r->registered_count = fz_scan(r->registered);
	fz_golden_load(r);
}

static void	account(t_fzrun *r, const t_fzcall *c, int64_t rc)
{
	if (c->num < FZ_SLOTS)
		r->by_num[c->num]++;
	else
		r->outside++;
	if (rc == E_NOSYS)
		r->nosys++;
	else if (rc >= 0)
		r->ok++;
	else
		r->refused++;
}

static void	fz_step(t_fzrun *r, const t_fzcall *c)
{
	int64_t		rc;
	bool		halted;
	const char	*bad;

	g_fp.ok_base = (uint64_t)(uintptr_t)r->buf;
	g_fp.ok_len = FZ_WRITABLE;
	g_fp.locks = 0;
	g_fp.unlocks = 0;
	g_fp.bad_locks = 0;
	rc = fz_call(c, &halted);
	bad = halted ? "panique ou arret du noyau" : fz_verdict(r, c, rc);
	if (bad)
	{
		if (r->incidents < FZ_REPORT_MAX)
			fz_report(r, c, rc, bad);
		r->incidents++;
	}
	account(r, c, rc);
	r->calls++;
}

static void	sweep_call(t_fzrun *r, t_fzcall *c, uint32_t variant)
{
	c->pid = 10 + variant % 3;
	c->flags = variant % 3 == 0 ? 0 : PF_DISPLAY | (variant % 5);
	c->no_proc = variant % 7 == 0;
	fz_step(r, c);
}

static void	sweep_fill(t_fzcall *c, uint64_t fill, uint32_t slot,
				uint64_t value)
{
	uint32_t	i;

	i = 0;
	while (i < 6)
		c->a[i++] = fill;
	if (slot < 6)
		c->a[slot] = value;
}

static void	sweep_number(t_fzrun *r, uint32_t num)
{
	t_fzcall	c;
	uint32_t	e;
	uint32_t	slot;
	uint32_t	v;

	memset(&c, 0, sizeof(c));
	c.num = num;
	v = 0;
	e = 0;
	while (e < sizeof(g_edges) / sizeof(g_edges[0]))
	{
		sweep_fill(&c, g_edges[e], 6, 0);
		sweep_call(r, &c, v++);
		slot = 0;
		while (slot < 6)
		{
			sweep_fill(&c, 0, slot, g_edges[e]);
			sweep_call(r, &c, v++);
			sweep_fill(&c, (uint64_t)(uintptr_t)r->buf, slot, g_edges[e]);
			sweep_call(r, &c, v++);
			slot++;
		}
		e++;
	}
}

bool	fz_sweep(t_fzrun *r)
{
	static const uint32_t	outside[FZ_OUTSIDE_COUNT] = {0x100, 0x160, 0x1000,
		0x80000060u, UINT32_MAX};
	uint32_t				num;

	num = 0;
	while (num < FZ_SLOTS)
		sweep_number(r, num++);
	num = 0;
	while (num < FZ_OUTSIDE_COUNT)
		sweep_number(r, outside[num++]);
	return (r->incidents == 0);
}

bool	fz_run(t_fzrun *r, uint64_t calls, uint64_t seconds)
{
	t_fzcall	c;
	time_t		stop;

	stop = time(NULL) + (time_t)seconds;
	while (seconds ? time(NULL) < stop : r->calls < calls)
	{
		fz_gen(r, &c);
		fz_step(r, &c);
	}
	return (r->incidents == 0);
}

static void	golden_line(t_fzrun *r, const char *line)
{
	char		name[64];
	char		status[16];
	unsigned	num;
	unsigned	last;
	int			args;

	if (sscanf(line, "trou %x %x", &num, &last) == 2)
	{
		while (num <= last && num < FZ_SLOTS)
			r->gstate[num++] = FZ_HOLE;
		return ;
	}
	if (sscanf(line, "%x %63s %15s %d", &num, name, status, &args) != 4
		|| num >= FZ_SLOTS || strncmp(name, "SYS_", 4) != 0)
		return ;
	r->gstate[num] = strcmp(status, "retire") == 0 ? FZ_RETIRED : FZ_ACTIVE;
	r->gargs[num] = (uint8_t)args;
	r->gowned[num] = strncmp(name, "SYS_DISPLAY_", 12) == 0
		|| strcmp(name, "SYS_KCON") == 0;
	r->gactive += r->gstate[num] == FZ_ACTIVE;
}

bool	fz_golden_load(t_fzrun *r)
{
	FILE	*f;
	char	line[160];

	memset(r->gstate, 0, sizeof(r->gstate));
	memset(r->gowned, 0, sizeof(r->gowned));
	memset(r->gargs, 0, sizeof(r->gargs));
	r->gactive = 0;
	f = fopen(FZ_GOLDEN_PATH, "r");
	if (!f)
		return (false);
	while (fgets(line, sizeof(line), f))
	{
		if (line[0] != '#')
			golden_line(r, line);
	}
	fclose(f);
	return (r->gactive > 0);
}

const char	*fz_check_table(const t_fzrun *r)
{
	uint32_t	n;

	n = 0;
	while (n < FZ_SLOTS)
	{
		if (fz_registered(n) && r->gstate[n] != FZ_ACTIVE)
			return ("appel enregistre hors de la liste figee ou retire");
		if (fz_registered(n) && !r->gowned[n])
			return ("appel enregistre qui n'appartient pas au lot");
		if (!fz_registered(n) && r->gowned[n])
			return ("appel du lot fige mais non enregistre");
		n++;
	}
	return (NULL);
}
