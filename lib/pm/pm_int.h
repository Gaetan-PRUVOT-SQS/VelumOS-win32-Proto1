#ifndef PM_INT_H
# define PM_INT_H

# include "velum/err.h"
# include "velum/libk.h"
# include "velum/apk/pm.h"

# define PM_LINE_MAX 460

typedef struct s_pmwork
{
	t_pmentry	sys[PM_MAX_PACKAGES];
	t_pmentry	data[PM_MAX_PACKAGES + 1];
	uint32_t	sys_n;
	uint32_t	data_n;
	uint32_t	slot;
	uint32_t	fresh;
	t_pminfo	info;
	char		text[PM_REG_MAX + 1];
	char		reg[PM_PATH_MAX];
	char		reg_tmp[PM_PATH_MAX];
	char		apk[PM_PATH_MAX];
	char		apk_tmp[PM_PATH_MAX];
}	t_pmwork;

void	*pm_alloc(size_t size);
void	pm_free(void *ptr);
size_t	pm_nlen(const char *s, size_t max);
int		pm_utf8_ok(const char *s, size_t n);
int		pm_pkg_ok(const char *s, size_t n);
int		pm_label_ok(const char *s, size_t n);
int		pm_act_ok(const char *s, size_t n);
int		pm_info_ok(const t_pminfo *i);
int		pm_same_pkg(const char *a, const char *b);
int		pm_dec_u32(const char *s, size_t n, uint32_t *out);
int		pm_hex_cert(const char *s, uint8_t *out);
int		pm_index(const t_pmentry *e, uint32_t n, const char *package);
int		pm_path(char *dst, const char *root, const char *name, const char *suf);
int		pm_work_open(t_pm *pm, t_pmwork **out);
int		pm_store(t_pm *pm, t_pmwork *w);
void	pm_drop(t_pm *pm, const char *path);

#endif
