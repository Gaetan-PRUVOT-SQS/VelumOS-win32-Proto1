#include <stdlib.h>
#include "velum/abi/abi_syscall.h"
#include "velum/err.h"
#include "fake_kern.h"
#include "ws_sys.h"

int	ws_sys_wait(const t_handle *hs, uint32_t n, uint64_t timeout_ns)
{
	uint32_t	i;
	int			obj;

	i = 0;
	while (i < n)
	{
		obj = fk_obj_of(hs[i], FK_FREE);
		if (obj < 0)
			return (E_BADF);
		if (fk_signaled(obj))
			return ((int)i);
		i++;
	}
	if (timeout_ns != TIMEOUT_INF)
		g_fk.now += timeout_ns;
	return (E_TIMEOUT);
}

int64_t	ws_sys_section(uint64_t size)
{
	int		obj;
	int64_t	h;

	if (size == 0 || size > 0x4000000ull)
		return (E_INVAL);
	if (fk_fail(&g_fk.fail_section))
		return (E_NOMEM);
	obj = fk_obj_new(FK_SECTION);
	if (obj < 0)
		return (obj);
	g_fk.o[obj].mem = calloc(1, size);
	g_fk.o[obj].size = size;
	h = fk_handle_new(obj);
	if (h < 0)
		fk_unref(obj);
	return (h);
}

void	*ws_sys_map(t_handle section, uint64_t len)
{
	return (fk_map(section, len));
}

void	ws_sys_unmap(void *va, uint64_t len)
{
	(void)len;
	fk_unmap(va);
}

int	ws_sys_display(t_dispinfo *di, void **fb)
{
	if (g_fk.fb == NULL)
		return (E_NODEV);
	di->width = g_fk.fb_w;
	di->height = g_fk.fb_h;
	di->pitch = g_fk.fb_pitch;
	di->bpp = 32;
	di->format = 1;
	di->flags = 0;
	di->fb_size = (uint64_t)g_fk.fb_pitch * g_fk.fb_h;
	*fb = g_fk.fb;
	return (0);
}
