#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "harness.h"
#include "velum/abi/abi_input.h"
#include "velum/abi/abi_ipc.h"
#include "velum/abi/abi_syscall.h"
#include "velum/abi/abi_types.h"
#include "velum/libk.h"

#define GOLDEN_PATH "tests/abi/syscalls.golden"
#define HEADER_PATH "include/velum/abi/abi_syscall.h"
#define WRAPPER_DIR "user/libvelum"
#define ENT_MAX 128
#define HOLE_MAX 32
#define SIZE_MAX_ENT 16
#define VIOL_MAX 64
#define NAME_MAX_LEN 48

_Static_assert(sizeof(t_vstat) == 40, "t_vstat");
_Static_assert(sizeof(t_dirent) == 272, "t_dirent");
_Static_assert(sizeof(t_procinfo) == 80, "t_procinfo");
_Static_assert(sizeof(t_sysinfo) == 160, "t_sysinfo");
_Static_assert(sizeof(t_dispinfo) == 32, "t_dispinfo");
_Static_assert(sizeof(t_dispmode) == 16, "t_dispmode");
_Static_assert(sizeof(t_chansend) == 32, "t_chansend");
_Static_assert(sizeof(t_chanrecv) == 56, "t_chanrecv");
_Static_assert(sizeof(t_inpevent) == 32, "t_inpevent");

typedef struct s_entry
{
	char		name[NAME_MAX_LEN];
	uint32_t	num;
	int			retired;
	int			args;
}	t_entry;

typedef struct s_hole
{
	uint32_t	first;
	uint32_t	last;
}	t_hole;

typedef struct s_sizeent
{
	char		name[NAME_MAX_LEN];
	uint32_t	size;
}	t_sizeent;

typedef struct s_abi
{
	t_entry		ent[ENT_MAX];
	t_hole		hole[HOLE_MAX];
	t_sizeent	size[SIZE_MAX_ENT];
	int			nent;
	int			nhole;
	int			nsize;
	uint32_t	version;
	uint32_t	max;
}	t_abi;

typedef struct s_report
{
	char	msg[VIOL_MAX][128];
	int		n;
}	t_report;

static void	rep_add(t_report *r, const char *fmt, const char *a, uint32_t n)
{
	if (r->n >= VIOL_MAX)
		return ;
	snprintf(r->msg[r->n], sizeof(r->msg[0]), fmt, a, n);
	r->n++;
}

static int	rep_has(const t_report *r, const char *needle)
{
	int	i;

	i = 0;
	while (i < r->n)
	{
		if (strstr(r->msg[i], needle))
			return (1);
		i++;
	}
	return (0);
}

static char	*slurp(const char *path)
{
	FILE	*f;
	char	*buf;
	long	n;

	f = fopen(path, "rb");
	if (!f)
		return (NULL);
	buf = NULL;
	if (fseek(f, 0, SEEK_END) == 0 && (n = ftell(f)) >= 0)
	{
		rewind(f);
		buf = calloc((size_t)n + 1, 1);
		if (buf && fread(buf, 1, (size_t)n, f) != (size_t)n)
		{
			free(buf);
			buf = NULL;
		}
	}
	fclose(f);
	return (buf);
}

static char	*replace_once(const char *text, const char *from, const char *to)
{
	const char	*at;
	char		*out;
	size_t		head;

	at = strstr(text, from);
	if (!at)
		return (NULL);
	head = (size_t)(at - text);
	out = calloc(strlen(text) + strlen(to) + 1, 1);
	if (!out)
		return (NULL);
	memcpy(out, text, head);
	memcpy(out + head, to, strlen(to));
	memcpy(out + head + strlen(to), at + strlen(from), strlen(at + strlen(from)));
	return (out);
}

static int	next_line(const char **cur, char *line, size_t cap)
{
	size_t	n;

	if (!**cur)
		return (0);
	n = 0;
	while ((*cur)[n] && (*cur)[n] != '\n')
		n++;
	if (n >= cap)
		n = cap - 1;
	memcpy(line, *cur, n);
	line[n] = '\0';
	*cur += n;
	while (**cur && **cur != '\n')
		(*cur)++;
	if (**cur == '\n')
		(*cur)++;
	return (1);
}

static t_entry	*find_name(t_abi *t, const char *name)
{
	int	i;

	i = 0;
	while (i < t->nent)
	{
		if (strcmp(t->ent[i].name, name) == 0)
			return (&t->ent[i]);
		i++;
	}
	return (NULL);
}

static t_entry	*find_num(t_abi *t, uint32_t num)
{
	int	i;

	i = 0;
	while (i < t->nent)
	{
		if (t->ent[i].num == num)
			return (&t->ent[i]);
		i++;
	}
	return (NULL);
}

static int	in_hole(const t_abi *t, uint32_t num)
{
	int	i;

	i = 0;
	while (i < t->nhole)
	{
		if (num >= t->hole[i].first && num <= t->hole[i].last)
			return (1);
		i++;
	}
	return (0);
}

static void	add_entry(t_abi *t, t_report *r, const t_entry *e)
{
	if (t->nent >= ENT_MAX)
		return (rep_add(r, "table pleine a %s", e->name, 0));
	if (find_name(t, e->name))
		rep_add(r, "nom en double : %s", e->name, 0);
	if (find_num(t, e->num))
		rep_add(r, "numero attribue deux fois : %s", e->name, e->num);
	t->ent[t->nent] = *e;
	t->nent++;
}

static void	golden_line(t_abi *t, t_report *r, const char *line)
{
	t_entry		e;
	t_hole		h;
	t_sizeent	s;
	char		status[16];

	memset(&e, 0, sizeof(e));
	if (sscanf(line, "version %u", &t->version) == 1
		|| sscanf(line, "max %x", &t->max) == 1)
		return ;
	if (sscanf(line, "trou %x %x", &h.first, &h.last) == 2 && t->nhole < HOLE_MAX)
		t->hole[t->nhole++] = h;
	else if (sscanf(line, "taille %47s %u", s.name, &s.size) == 2
		&& t->nsize < SIZE_MAX_ENT)
		t->size[t->nsize++] = s;
	else if (sscanf(line, "%x %47s %15s %d", &e.num, e.name, status,
			&e.args) == 4)
	{
		e.retired = strcmp(status, "retire") == 0;
		if (!e.retired && strcmp(status, "actif") != 0)
			rep_add(r, "statut inconnu : %s", e.name, 0);
		add_entry(t, r, &e);
	}
	else
		rep_add(r, "ligne illisible : %s", line, 0);
}

static void	parse_golden(const char *text, t_abi *t, t_report *r)
{
	char	line[160];

	memset(t, 0, sizeof(*t));
	while (next_line(&text, line, sizeof(line)))
	{
		if (line[0] == '#' || line[0] == '\0')
			continue ;
		golden_line(t, r, line);
	}
}

static void	parse_header(const char *text, t_abi *t, t_report *r)
{
	char	line[160];
	t_entry	e;

	memset(t, 0, sizeof(*t));
	while (next_line(&text, line, sizeof(line)))
	{
		memset(&e, 0, sizeof(e));
		if (sscanf(line, "# define VELUM_ABI_VERSION %u", &t->version) == 1)
			continue ;
		if (sscanf(line, "# define SYS_MAX %x", &t->max) == 1)
			continue ;
		if (sscanf(line, "# define %47s %x", e.name, &e.num) == 2
			&& strncmp(e.name, "SYS_", 4) == 0)
			add_entry(t, r, &e);
	}
}

static int	wrapper_args_six(const char *text, const char *call)
{
	const char	*start;
	const char	*cur;
	char		key[32];
	int			last;
	int			k;

	start = call;
	while (start > text && strncmp(start, "a[0] =", 6) != 0)
		start--;
	last = -1;
	k = 0;
	while (k < 6)
	{
		snprintf(key, sizeof(key), "a[%d] = ", k);
		cur = strstr(start, key);
		if (!cur || cur > call)
			break ;
		if (strncmp(cur + strlen(key), "0;", 2) != 0)
			last = k;
		k++;
	}
	return (last + 1);
}

static void	wrapper_add(t_abi *w, t_report *r, const char *name, int args)
{
	t_entry	*known;
	t_entry	e;

	known = find_name(w, name);
	if (known)
	{
		if (known->args != args)
			rep_add(r, "wrapper contradictoire : %s", name, 0);
		return ;
	}
	memset(&e, 0, sizeof(e));
	snprintf(e.name, sizeof(e.name), "%s", name);
	e.args = args;
	e.num = (uint32_t)w->nent;
	if (w->nent < ENT_MAX)
		w->ent[w->nent++] = e;
}

static void	parse_wrappers(const char *text, t_abi *w, t_report *r)
{
	const char	*p;
	char		name[NAME_MAX_LEN];
	size_t		len;
	int			args;

	memset(w, 0, sizeof(*w));
	p = text;
	while ((p = strstr(p + 1, "(SYS_")) != NULL)
	{
		len = strspn(p + 1, "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_");
		if (len >= sizeof(name))
			continue ;
		memcpy(name, p + 1, len);
		name[len] = '\0';
		args = -1;
		if (p - text >= 4 && strncmp(p - 4, "sys", 3) == 0
			&& p[-1] >= '0' && p[-1] <= '9')
			args = p[-1] - '0';
		else if (p - text >= 10 && strncmp(p - 10, "v_syscall6", 10) == 0)
			args = wrapper_args_six(text, p);
		if (args >= 0)
			wrapper_add(w, r, name, args);
	}
}

static void	check_meta(t_abi *g, t_abi *h, t_report *r)
{
	if (g->version != h->version)
		rep_add(r, "version ABI changee (%s) : %u", "VELUM_ABI_VERSION",
			h->version);
	if (g->max != h->max)
		rep_add(r, "SYS_MAX change (%s) : %u", "SYS_MAX", h->max);
}

static void	check_golden_entry(t_abi *h, t_report *r, const t_entry *g)
{
	t_entry	*byname;
	t_entry	*bynum;

	byname = find_name(h, g->name);
	bynum = find_num(h, g->num);
	if (g->retired)
	{
		if (byname)
			rep_add(r, "retire mais encore defini : %s", g->name, 0);
		if (bynum)
			rep_add(r, "numero retire reattribue a %s", bynum->name, g->num);
		return ;
	}
	if (!byname)
		rep_add(r, "appel disparu sans retrait : %s", g->name, g->num);
	else if (byname->num != g->num)
		rep_add(r, "numero change : %s", g->name, byname->num);
	if (bynum && strcmp(bynum->name, g->name) != 0)
		rep_add(r, "nom change ou numero reutilise : %s", bynum->name, g->num);
}

static void	check_entries(t_abi *g, t_abi *h, t_report *r)
{
	int	i;

	i = 0;
	while (i < g->nent)
		check_golden_entry(h, r, &g->ent[i++]);
	i = 0;
	while (i < h->nent)
	{
		if (!find_name(g, h->ent[i].name) && !find_num(g, h->ent[i].num))
			rep_add(r, "appel absent du golden : %s", h->ent[i].name,
				h->ent[i].num);
		if (in_hole(g, h->ent[i].num))
			rep_add(r, "numero dans un trou declare : %s", h->ent[i].name,
				h->ent[i].num);
		i++;
	}
}

static void	check_coverage(t_abi *g, t_report *r)
{
	uint32_t	n;
	int			used;
	int			hole;

	n = 0;
	while (n < g->max)
	{
		used = find_num(g, n) != NULL;
		hole = in_hole(g, n);
		if (!used && !hole)
			rep_add(r, "numero ni attribue ni declare en trou%s", "", n);
		if (used && hole)
			rep_add(r, "numero attribue et declare en trou%s", "", n);
		n++;
	}
}

static void	check_args(t_abi *g, t_abi *w, t_report *r)
{
	t_entry	*wrapper;
	int		i;

	i = 0;
	while (i < g->nent)
	{
		wrapper = find_name(w, g->ent[i].name);
		if (!g->ent[i].retired && !wrapper)
			rep_add(r, "aucun wrapper libvelum : %s", g->ent[i].name, 0);
		else if (!g->ent[i].retired && wrapper->args != g->ent[i].args)
			rep_add(r, "nombre d'arguments change : %s", g->ent[i].name,
				(uint32_t)wrapper->args);
		i++;
	}
}

static const t_sizeent	g_current_sizes[] = {
{"t_vstat", sizeof(t_vstat)}, {"t_dirent", sizeof(t_dirent)},
{"t_procinfo", sizeof(t_procinfo)}, {"t_sysinfo", sizeof(t_sysinfo)},
{"t_dispinfo", sizeof(t_dispinfo)}, {"t_dispmode", sizeof(t_dispmode)},
{"t_chansend", sizeof(t_chansend)}, {"t_chanrecv", sizeof(t_chanrecv)},
{"t_inpevent", sizeof(t_inpevent)}, {"", 0}
};

static void	check_sizes(t_abi *g, t_report *r)
{
	int	i;
	int	j;
	int	found;

	i = 0;
	while (g_current_sizes[i].name[0])
	{
		found = 0;
		j = 0;
		while (j < g->nsize)
		{
			if (strcmp(g->size[j].name, g_current_sizes[i].name) == 0)
			{
				found = 1;
				if (g->size[j].size != g_current_sizes[i].size)
					rep_add(r, "taille changee : %s", g->size[j].name,
						g_current_sizes[i].size);
			}
			j++;
		}
		if (!found)
			rep_add(r, "structure absente du golden : %s",
				g_current_sizes[i].name, 0);
		i++;
	}
}

static void	compare(const char *golden, const char *header,
	const char *wrappers, t_report *r)
{
	static t_abi	g;
	static t_abi	h;
	static t_abi	w;

	r->n = 0;
	parse_golden(golden, &g, r);
	parse_header(header, &h, r);
	parse_wrappers(wrappers, &w, r);
	check_meta(&g, &h, r);
	check_entries(&g, &h, r);
	check_coverage(&g, r);
	check_args(&g, &w, r);
	check_sizes(&g, r);
}

static char	*load_wrappers(void)
{
	DIR				*d;
	struct dirent	*de;
	char			path[512];
	char			*all;
	char			*one;
	size_t			len;

	d = opendir(WRAPPER_DIR);
	all = calloc(1, 1);
	len = 0;
	while (d && all && (de = readdir(d)) != NULL)
	{
		if (strncmp(de->d_name, "sys_", 4) != 0 || !strstr(de->d_name, ".c"))
			continue ;
		snprintf(path, sizeof(path), "%s/%s", WRAPPER_DIR, de->d_name);
		one = slurp(path);
		if (!one)
			continue ;
		all = realloc(all, len + strlen(one) + 2);
		memcpy(all + len, one, strlen(one) + 1);
		len += strlen(one);
		free(one);
	}
	if (d)
		closedir(d);
	return (all);
}

static char	*g_golden;
static char	*g_header;
static char	*g_wrappers;

static void	show(const t_report *r)
{
	int	i;

	i = 0;
	while (i < r->n)
		fprintf(stderr, "  violation : %s\n", r->msg[i++]);
}

static void	abi_reel(void)
{
	static t_report	r;

	compare(g_golden, g_header, g_wrappers, &r);
	show(&r);
	h_eq_i64("table courante conforme au golden", r.n, 0);
}

static void	mutate(const char *const *edit, const char *expect)
{
	static t_report	r;
	char			*golden;
	char			*header;

	golden = g_golden;
	header = g_header;
	if (edit[0])
		golden = replace_once(g_golden, edit[0], edit[1]);
	if (edit[2])
		header = replace_once(g_header, edit[2], edit[3]);
	h_true(golden != NULL && header != NULL, "mutation applicable");
	if (golden && header)
	{
		compare(golden, header, g_wrappers, &r);
		h_true(r.n > 0, expect);
		h_true(rep_has(&r, expect), expect);
	}
	if (golden != g_golden)
		free(golden);
	if (header != g_header)
		free(header);
}

static void	mutate_golden(const char *from, const char *to, const char *expect)
{
	const char	*edit[4];

	edit[0] = from;
	edit[1] = to;
	edit[2] = NULL;
	edit[3] = NULL;
	mutate(edit, expect);
}

static void	mutate_header(const char *from, const char *to, const char *expect)
{
	const char	*edit[4];

	edit[0] = NULL;
	edit[1] = NULL;
	edit[2] = from;
	edit[3] = to;
	mutate(edit, expect);
}

static void	mutate_both(const char *const *edit, const char *expect)
{
	mutate(edit, expect);
}

static void	mutate_clean(const char *const *edit)
{
	static t_report	r;
	char			*golden;
	char			*header;

	golden = replace_once(g_golden, edit[0], edit[1]);
	header = replace_once(g_header, edit[2], edit[3]);
	h_true(golden != NULL && header != NULL, "mutation legitime applicable");
	if (golden && header)
	{
		compare(golden, header, g_wrappers, &r);
		show(&r);
		h_eq_i64("retrait legitime sans violation", r.n, 0);
	}
	free(golden);
	free(header);
}

static void	mutations_reutilisation(void)
{
	mutate_header("# define SYS_MAX 0xff", "# define SYS_NOUVEAU 0x07\n"
		"# define SYS_MAX 0xff", "attribue deux fois");
	mutate_header("SYS_GETTID 0x07", "SYS_AUTRE 0x07",
		"nom change ou numero reutilise");
	mutate_header("SYS_GETTID 0x07", "SYS_GETTID 0x06", "attribue deux fois");
	mutate_header("SYS_READ 0x31", "SYS_READ 0x32", "numero change");
	mutate_header("SYS_YIELD 0x50", "SYS_YIELD 0x55", "dans un trou declare");
	mutate_header("# define SYS_MAX 0xff", "# define SYS_READ 0x3e\n"
		"# define SYS_MAX 0xff", "nom en double");
	mutate_header("# define SYS_MAX 0xff", "# define SYS_NOUVEAU 0xff\n"
		"# define SYS_MAX 0xff", "absent du golden");
	mutate_golden("0x31 SYS_READ actif 3\n", "", "absent du golden");
	mutate_golden("0x51 SYS_SLEEP", "0x51 SYS_YIELD", "nom en double");
	mutate_golden("0x50 SYS_YIELD actif", "0x50 SYS_YIELD suspendu",
		"statut inconnu");
	mutate_golden("0x50 SYS_YIELD actif 0", "0x50 SYS_YIELD", "illisible");
	mutate_clean((const char *const [4]){"0x50 SYS_YIELD actif 0",
		"0x50 SYS_YIELD retire 0", "# define SYS_YIELD 0x50\n", ""});
}

static void	mutations_numeros(void)
{
	mutate_golden("0x02 SYS_PROC_KILL", "0x01 SYS_PROC_KILL",
		"attribue deux fois");
	mutate_header("SYS_GETTID 0x07", "SYS_GETTID 0x0b", "numero change");
	mutate_header("# define SYS_LOG 0x0a\n", "", "disparu sans retrait");
	mutate_header("SYS_YIELD 0x50", "SYS_AUTRE 0x50", "reutilise");
	mutate_header("# define SYS_MAX 0xff", "# define SYS_MAX 0x7f",
		"SYS_MAX change");
	mutate_header("VELUM_ABI_VERSION 1", "VELUM_ABI_VERSION 2",
		"version ABI changee");
	mutate_header("# define SYS_MAX 0xff", "# define SYS_NOUVEAU 0x18\n"
		"# define SYS_MAX 0xff", "absent du golden");
}

static void	mutations_retrait(void)
{
	mutate_golden("0x50 SYS_YIELD actif", "0x50 SYS_YIELD retire",
		"retire mais encore defini");
	mutate_both((const char *const [4]){"0x50 SYS_YIELD actif 0",
		"0x50 SYS_YIELD retire 0", "SYS_YIELD 0x50", "SYS_AUTRE 0x50"},
		"numero retire reattribue a SYS_AUTRE");
	mutate_golden("trou 0x18 0x18\n", "", "ni attribue ni declare");
	mutate_golden("trou 0x18 0x18\n", "trou 0x18 0x18\ntrou 0x10 0x10\n",
		"attribue et declare en trou");
	mutate_header("# define SYS_MAX 0xff", "# define SYS_NOUVEAU 0x20\n"
		"# define SYS_MAX 0xff", "dans un trou declare");
}

static void	mutations_contrat(void)
{
	mutate_golden("SYS_READ actif 3", "SYS_READ actif 2",
		"nombre d'arguments change : SYS_READ");
	mutate_golden("taille t_vstat 40", "taille t_vstat 48", "taille changee");
	mutate_golden("taille t_chanrecv 56\n", "", "structure absente");
	mutate_golden("0x31 SYS_READ", "0x31 SYS_LIRE", "disparu sans retrait");
	mutate_golden("0x31 SYS_READ", "0x31 SYS_LIRE", "nom change");
}

int	main(int argc, char **argv)
{
	static t_report	r;
	char			*file;

	g_golden = slurp(GOLDEN_PATH);
	g_header = slurp(HEADER_PATH);
	g_wrappers = load_wrappers();
	h_begin("a07/contrat_abi_golden");
	if (!g_golden || !g_header || !g_wrappers)
		h_true(0, "lecture du golden, de l'en-tete ou des wrappers");
	else if (argc > 1)
	{
		file = slurp(argv[1]);
		h_true(file != NULL, "lecture du golden fourni");
		if (file)
		{
			compare(file, g_header, g_wrappers, &r);
			show(&r);
			h_eq_i64("golden fourni conforme", r.n, 0);
		}
		free(file);
	}
	else
	{
		h_run("table courante", abi_reel);
		h_run("mutation reutilisation et suppression",
			mutations_reutilisation);
		h_run("mutation numeros", mutations_numeros);
		h_run("mutation retraits et trous", mutations_retrait);
		h_run("mutation arguments noms tailles", mutations_contrat);
	}
	free(g_golden);
	free(g_header);
	free(g_wrappers);
	return (h_end());
}
