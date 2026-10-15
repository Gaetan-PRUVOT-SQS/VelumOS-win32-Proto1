#include <stdlib.h>
#include "velum/err.h"
#include "velum/libk.h"
#include "../common/apkglue.h"
#include "../common/platform.h"
#include "../common/tbuf.h"
#include "apkinst.h"

static const char	*rule_reason(int err)
{
	if (err == E_PERM)
		return ("paquet préinstallé, jamais remplacé");
	if (err == E_NOSPC)
		return ("trop de paquets installés");
	if (err == E_EXIST)
		return ("un paquet du même nom à la casse près existe déjà");
	if (err == E_ACCES)
		return ("signataire différent de la version installée");
	if (err == E_RANGE)
		return ("fichier trop gros");
	if (err == E_NOMEM)
		return ("mémoire insuffisante");
	if (err == E_NOENT)
		return ("fichier introuvable");
	if (err == E_INVAL)
		return ("paquet invalide ou version plus ancienne que l'installée");
	return ("écriture impossible sous " APKINST_DATA_ROOT);
}

const char	*apkinst_reason(int err, int apk_reason_code)
{
	if (apk_reason_code > APKR_OK && apk_reason_code < APKR_COUNT)
		return (apk_reason(apk_reason_code));
	return (rule_reason(err));
}

int	apkinst_install(const char *path, t_pmentry *out)
{
	uint8_t	*file;
	int64_t	n;
	t_pm	pm;
	int		r;

	n = apkglue_read_file(path, &file);
	if (n < 0)
		return ((int)n);
	pm_defaults(&pm);
	pm.sys_root = APKINST_SYS_ROOT;
	pm.data_root = APKINST_DATA_ROOT;
	pm.fs = apkglue_fs();
	pm.inspect = apkglue_inspect;
	pm.ctx = NULL;
	r = pm_install(&pm, (t_span){file, (size_t)n}, out);
	free(file);
	return (r);
}

void	apkinst_report(int err, const t_pmentry *e)
{
	char	line[APKINST_LINE_MAX];
	t_tbuf	b;

	tb_init(&b, line, sizeof(line));
	tb_str(&b, "apkinst: ");
	if (err >= 0)
	{
		tb_str(&b, e->info.package);
		tb_str(&b, " installé");
	}
	else
	{
		tb_str(&b, "refusé : ");
		tb_str(&b, apkinst_reason(err, apkglue_reason()));
	}
	os_log(line);
}
