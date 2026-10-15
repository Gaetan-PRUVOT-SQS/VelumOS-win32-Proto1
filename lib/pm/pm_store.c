#include "pm_int.h"

void	pm_drop(t_pm *pm, const char *path)
{
	(void)pm->fs->unlink(pm->ctx, path);
}

int	pm_store(t_pm *pm, t_pmwork *w)
{
	t_text	out;
	t_span	data;
	int		n;

	out.p = w->text;
	out.cap = (size_t)pm->max_registry + 1;
	n = pm_registry_format(w->data, w->data_n, out);
	if (n < 0)
		return (n);
	data.p = (const uint8_t *)w->text;
	data.len = (size_t)n;
	pm_drop(pm, w->reg_tmp);
	n = pm->fs->write_new(pm->ctx, w->reg_tmp, data);
	if (n >= 0)
		n = pm->fs->rename(pm->ctx, w->reg_tmp, w->reg);
	if (n < 0)
	{
		pm_drop(pm, w->reg_tmp);
		return (n);
	}
	return (0);
}
