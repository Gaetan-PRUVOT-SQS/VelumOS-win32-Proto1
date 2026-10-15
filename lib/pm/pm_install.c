#include "pm_int.h"

static int	examine(t_pm *pm, t_pmwork *w, t_span apk)
{
	t_pminfo	raw;
	int			err;

	if (!apk.p || apk.len == 0)
		return (E_INVAL);
	if (apk.len > pm->max_apk)
		return (E_RANGE);
	memset(&raw, 0, sizeof(raw));
	err = pm->inspect(apk, &raw);
	if (err < 0)
		return (err);
	if (!pm_info_ok(&raw))
		return (E_INVAL);
	memset(&w->info, 0, sizeof(w->info));
	memcpy(w->info.package, raw.package, pm_nlen(raw.package, PM_PKG_MAX));
	memcpy(w->info.label, raw.label, pm_nlen(raw.label, PM_LABEL_MAX));
	memcpy(w->info.activity, raw.activity, pm_nlen(raw.activity, PM_ACT_MAX));
	memcpy(w->info.cert, raw.cert, PM_CERT_LEN);
	w->info.version_code = raw.version_code;
	return (0);
}

static int	check_rules(t_pm *pm, t_pmwork *w)
{
	int	i;

	if (pm_index(w->sys, w->sys_n, w->info.package) >= 0)
		return (E_PERM);
	i = pm_index(w->data, w->data_n, w->info.package);
	w->fresh = (i < 0);
	w->slot = w->data_n;
	if (i < 0 && w->sys_n + w->data_n >= pm->max_packages)
		return (E_NOSPC);
	if (i < 0)
		return (0);
	w->slot = (uint32_t)i;
	if (memcmp(w->data[i].info.package, w->info.package, PM_PKG_MAX) != 0)
		return (E_EXIST);
	if (memcmp(w->data[i].info.cert, w->info.cert, PM_CERT_LEN) != 0)
		return (E_ACCES);
	if (w->info.version_code < w->data[i].info.version_code)
		return (E_INVAL);
	return (0);
}

static int	publish(t_pm *pm, t_pmwork *w)
{
	int	err;

	memset(&w->data[w->slot], 0, sizeof(w->data[w->slot]));
	w->data[w->slot].info = w->info;
	w->data[w->slot].origin = PM_ORIGIN_DATA;
	memcpy(w->data[w->slot].path, w->apk, PM_PATH_MAX);
	w->data_n += w->fresh;
	err = pm_store(pm, w);
	if (err < 0 && w->fresh)
		pm_drop(pm, w->apk);
	return (err);
}

static int	commit(t_pm *pm, t_pmwork *w, t_span apk)
{
	int	err;

	if (pm_path(w->apk, pm->data_root, w->info.package, ".apk") < 0
		|| pm_path(w->apk_tmp, pm->data_root, w->info.package, ".apk.tmp") < 0
		|| pm_path(w->reg_tmp, pm->data_root, PM_REG_NAME, ".tmp") < 0)
		return (E_INVAL);
	err = pm->fs->mkdir(pm->ctx, pm->data_root);
	if (err < 0 && err != E_EXIST)
		return (err);
	pm_drop(pm, w->apk_tmp);
	err = pm->fs->write_new(pm->ctx, w->apk_tmp, apk);
	if (err >= 0)
		err = pm->fs->rename(pm->ctx, w->apk_tmp, w->apk);
	if (err < 0)
	{
		pm_drop(pm, w->apk_tmp);
		return (err);
	}
	return (publish(pm, w));
}

int	pm_install(t_pm *pm, t_span apk, t_pmentry *out)
{
	t_pmwork	*w;
	int			err;

	err = pm_work_open(pm, &w);
	if (err < 0)
		return (err);
	err = examine(pm, w, apk);
	if (err >= 0)
		err = check_rules(pm, w);
	if (err >= 0)
		err = commit(pm, w, apk);
	if (err >= 0 && out)
		*out = w->data[w->slot];
	pm_free(w);
	if (err < 0)
		return (err);
	return (0);
}
