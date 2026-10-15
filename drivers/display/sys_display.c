#include "display_int.h"
#include "velum/err.h"
#include "velum/vmm.h"

int	display_priv(void)
{
	t_process	*p;

	p = proc_current();
	if (!p || !(p->flags & PF_DISPLAY))
		return (E_PERM);
	return (E_OK);
}

int64_t	sys_display_info(const t_sysargs *a)
{
	t_dispinfo	info;

	if (!g_display.ready)
		return (E_NODEV);
	mutex_lock(&g_display.lock);
	info = g_display.info;
	mutex_unlock(&g_display.lock);
	return (copy_to_user((t_uptr)a->a[0], &info, sizeof(info)));
}

int64_t	sys_display_modes(const t_sysargs *a)
{
	t_dispmode	buf[DISP_MODES_MAX];
	uint32_t	max;
	uint32_t	n;
	int			rc;

	if (a->a[1] > UINT32_MAX)
		return (E_INVAL);
	if (!g_display.ready)
		return (E_NODEV);
	max = (uint32_t)a->a[1];
	if (max > DISP_MODES_MAX)
		max = DISP_MODES_MAX;
	n = display_modes(buf, max);
	if (max == 0 || n == 0)
		return ((int64_t)n);
	rc = copy_to_user((t_uptr)a->a[0], buf, n * sizeof(buf[0]));
	if (rc < 0)
		return (rc);
	return ((int64_t)n);
}

int64_t	sys_display_set_mode(const t_sysargs *a)
{
	int	rc;

	rc = display_priv();
	if (rc < 0)
		return (rc);
	if (a->a[0] > UINT32_MAX || a->a[1] > UINT32_MAX || a->a[2] > UINT32_MAX)
		return (E_INVAL);
	return (display_set_mode((uint32_t)a->a[0], (uint32_t)a->a[1],
			(uint32_t)a->a[2]));
}

int64_t	sys_kcon(const t_sysargs *a)
{
	int	rc;

	rc = display_priv();
	if (rc < 0)
		return (rc);
	if (a->a[0] > 1)
		return (E_INVAL);
	if (!g_display.ready)
		return (E_NODEV);
	kcon_enable(a->a[0] == 1);
	if (a->a[0] == 1 && !kcon_enabled())
		return (E_NOTSUP);
	return (E_OK);
}
