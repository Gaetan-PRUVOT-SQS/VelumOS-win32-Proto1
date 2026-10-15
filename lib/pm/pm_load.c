#include "pm_int.h"

static int	pm_valid(const t_pm *pm)
{
	return (pm && pm->fs && pm->inspect && pm->sys_root && pm->data_root
		&& pm->fs->read && pm->fs->write_new && pm->fs->rename
		&& pm->fs->unlink && pm->fs->mkdir && pm->max_packages > 0
		&& pm->max_packages <= PM_MAX_PACKAGES && pm->max_apk > 0
		&& pm->max_apk <= PM_APK_MAX && pm->max_registry > 0
		&& pm->max_registry <= PM_REG_MAX);
}

static int	read_reg(t_pm *pm, t_pmwork *w, const char *root, t_pmreg *r)
{
	int64_t	got;
	t_text	buf;
	t_span	text;

	if (pm_path(w->reg, root, PM_REG_NAME, "") < 0)
		return (E_INVAL);
	buf.p = w->text;
	buf.cap = pm->max_registry;
	got = pm->fs->read(pm->ctx, w->reg, buf);
	if (got == E_NOENT)
		got = 0;
	if (got < 0)
		return ((int)got);
	if ((uint64_t)got > pm->max_registry)
		return (E_OVERFLOW);
	text.p = (const uint8_t *)w->text;
	text.len = (size_t)got;
	return (pm_registry_scan(text, r));
}

static void	stamp(t_pmreg *r, const char *root, uint32_t origin)
{
	uint32_t	i;

	i = 0;
	while (i < r->n)
	{
		r->e[i].origin = origin;
		(void)pm_path(r->e[i].path, root, r->e[i].info.package, ".apk");
		i++;
	}
}

static int	load_root(t_pm *pm, t_pmwork *w, uint32_t origin)
{
	t_pmreg		r;
	const char	*root;
	int			err;

	root = pm->sys_root;
	r.e = w->sys;
	if (origin == PM_ORIGIN_DATA)
	{
		root = pm->data_root;
		r.e = w->data;
	}
	r.max = pm->max_packages;
	err = read_reg(pm, w, root, &r);
	if (err < 0)
		return (err);
	stamp(&r, root, origin);
	if (origin == PM_ORIGIN_DATA)
		w->data_n = r.n;
	else
		w->sys_n = r.n;
	return (0);
}

int	pm_work_open(t_pm *pm, t_pmwork **out)
{
	t_pmwork	*w;
	int			err;

	*out = NULL;
	if (!pm_valid(pm))
		return (E_INVAL);
	w = pm_alloc(sizeof(*w));
	if (!w)
		return (E_NOMEM);
	err = load_root(pm, w, PM_ORIGIN_SYSTEM);
	if (err == 0)
		err = load_root(pm, w, PM_ORIGIN_DATA);
	if (err < 0)
	{
		pm_free(w);
		return (err);
	}
	*out = w;
	return (0);
}
