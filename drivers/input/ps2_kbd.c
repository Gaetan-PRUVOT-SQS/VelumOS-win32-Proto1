#include "ps2.h"
#include "velum/err.h"
#include "velum/klog.h"

int	ps2_kbd_init(t_ps2 *g)
{
	uint8_t	id;
	int		rc;

	rc = ps2_dev_reset(g, PS2_PORT_KBD, &id);
	if (rc < 0)
	{
		klog_info("input: clavier PS/2 absent (%d)", rc);
		return (rc);
	}
	rc = ps2_dev_cmd_arg(g, PS2_PORT_KBD, PS2_K_SCANSET, 2);
	if (rc < 0)
		klog_warn("input: jeu de codes 2 non confirmé (%d)", rc);
	rc = ps2_dev_cmd(g, PS2_PORT_KBD, PS2_D_ENABLE);
	if (rc < 0)
	{
		klog_warn("input: clavier PS/2 sans activation (%d)", rc);
		return (rc);
	}
	scan2_reset(&g->scan);
	klog_info("input: clavier PS/2 prêt");
	return (0);
}
