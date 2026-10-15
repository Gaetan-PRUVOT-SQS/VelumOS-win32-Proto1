#include "fake.h"

static int	f_mkdir(void *ctx, const char *path)
{
	(void)ctx;
	(void)path;
	if (fake_fs_tick() < 0)
		return (E_IO);
	return (E_EXIST);
}

void	fake_setup(t_pm *pm)
{
	static t_pmfs	table;

	fake_fs_reset();
	fake_mem_reset(0);
	memset(&g_insp, 0, sizeof(g_insp));
	table = *fake_fs_table();
	table.mkdir = f_mkdir;
	memset(pm, 0, sizeof(*pm));
	pm->sys_root = FAKE_SYS;
	pm->data_root = FAKE_USR;
	pm->fs = &table;
	pm->inspect = fake_inspect;
	pm_defaults(pm);
}

int	fake_install(t_pm *pm, const char *pkg, uint32_t v, uint8_t s)
{
	uint8_t	body[8];
	t_span	apk;

	fake_app(pkg, v, s);
	memset(body, s, sizeof(body));
	body[0] = (uint8_t)v;
	apk.p = body;
	apk.len = sizeof(body);
	return (pm_install(pm, apk, NULL));
}

void	fake_reg_text(char *dst)
{
	t_ffile	*f;

	dst[0] = '\0';
	f = fake_fs_get(FAKE_REG);
	if (!f || f->len >= FAKE_DATA)
		return ;
	memcpy(dst, f->data, f->len);
	dst[f->len] = '\0';
}

int	fake_listed_ok(t_pm *pm)
{
	static t_pmentry	e[PM_MAX_PACKAGES];
	int					n;
	int					i;

	g_fs.fail_at = 0;
	n = pm_list(pm, e, PM_MAX_PACKAGES);
	i = 0;
	while (i < n)
	{
		if (e[i].origin == PM_ORIGIN_DATA && !fake_fs_get(e[i].path))
			return (-1);
		i++;
	}
	return (n);
}
