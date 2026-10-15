#include <stdint.h>
#include "harness.h"
#include "velum/err.h"
#include "velum/libk.h"
#include "startmenu.h"

static const char	*g_bad_arg[] = {
	"A;/system/bin/a;\n",
	"A;/system/bin/a;/data/x.apk\n",
	"A;/system/bin/a;x.apk\n",
	"A;/system/bin/a;/system/../x.apk\n",
	"A;/system/bin/a;/system/apps/a b.apk\n",
	"A;/system/bin/a;/system/apps/\n",
	"A;/system/bin/a;/system/a;/system/b\n",
	"A;/system/bin/a;/system/apps/\xc3\xa9.apk\n"
};

static size_t	put_full(char *dst, size_t arg_n)
{
	memset(dst, 'N', 31);
	dst[31] = ';';
	memset(dst + 32, 'p', 63);
	memcpy(dst + 32, SM_PROG_ROOT, SM_PROG_ROOT_LEN);
	dst[95] = ';';
	memset(dst + 96, 'a', arg_n);
	memcpy(dst + 96, SM_PROG_ROOT, SM_PROG_ROOT_LEN);
	dst[96 + arg_n] = '\n';
	return (97 + arg_n);
}

static void	argument_accepte(void)
{
	t_smprogs	p;
	const char	*line;

	line = "Bonjour APK;/system/bin/apkrun;/system/apps/b.apk\nB;/system/b\n";
	h_eq_i64("deux lignes", sm_progs_parse(line, strlen(line), &p), 2);
	h_eq_str("nom", p.list[0].name, "Bonjour APK");
	h_eq_str("chemin", p.list[0].path, "/system/bin/apkrun");
	h_eq_str("argument", p.list[0].arg, "/system/apps/b.apk");
	h_eq_str("ligne sans argument", p.list[1].arg, "");
}

static void	argument_hostile(void)
{
	t_smprogs	p;
	size_t		i;

	i = 0;
	while (i < sizeof(g_bad_arg) / sizeof(g_bad_arg[0]))
	{
		p.count = 9;
		h_eq_i64(g_bad_arg[i], sm_progs_parse(g_bad_arg[i],
				strlen(g_bad_arg[i]), &p), E_INVAL);
		h_eq_u64("table videe", p.count, 0);
		i++;
	}
}

static void	argument_valeurs_limites(void)
{
	char		buf[SM_PROGS_FILE_MAX + 8];
	t_smprogs	p;
	size_t		n;

	h_eq_i64("argument 63", sm_progs_parse(buf, put_full(buf, 63), &p), 1);
	h_eq_u64("argument 63 copie", strlen(p.list[0].arg), 63);
	h_eq_i64("argument 64", sm_progs_parse(buf, put_full(buf, 64), &p),
		E_INVAL);
	h_eq_i64("argument 9", sm_progs_parse(buf, put_full(buf, 9), &p), 1);
	h_eq_i64("argument 8", sm_progs_parse(buf, put_full(buf, 8), &p),
		E_INVAL);
	n = put_full(buf, 63);
	n += put_full(buf + n, 63);
	n += put_full(buf + n, 63);
	h_eq_u64("taille maximale", n, SM_PROGS_FILE_MAX);
	h_eq_i64("fichier plein", sm_progs_parse(buf, n, &p), SM_PROGS_MAX);
	buf[n] = '\n';
	h_eq_i64("un octet de trop", sm_progs_parse(buf, n + 1, &p), E_INVAL);
}

int	main(void)
{
	h_begin("a20/programmes-arg");
	h_run("programmes: argument accepte", argument_accepte);
	h_run("programmes: argument hostile", argument_hostile);
	h_run("programmes: argument, valeurs limites", argument_valeurs_limites);
	return (h_end());
}
