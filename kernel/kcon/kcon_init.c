#include "../../drivers/display/display_int.h"
#include "kcon_int.h"
#include "velum/klog.h"

t_kcon	g_kcon;

void	kcon_attach(const t_surface *s)
{
	int	rc;

	rc = kcon_setup(&g_kcon, s, font_get(FONT_MONO));
	if (rc < 0)
		klog_warn("kcon: console indisponible (%d)", rc);
}
