#include "ps2.h"
#include "velum/err.h"
#include "velum/klog.h"

static int	mouse_rate(t_ps2 *g, uint8_t rate)
{
	return (ps2_dev_cmd_arg(g, PS2_PORT_MOUSE, PS2_M_SAMPLE, rate));
}

static int	mouse_read_id(t_ps2 *g, uint8_t *id)
{
	int	rc;

	rc = ps2_dev_cmd(g, PS2_PORT_MOUSE, PS2_M_GETID);
	if (rc < 0)
		return (rc);
	return (ps2_data_read(g->ops, id, PS2_T_DEV));
}

static int	mouse_detect_wheel(t_ps2 *g, uint8_t *id)
{
	int	rc;

	rc = mouse_rate(g, 200);
	if (rc == 0)
		rc = mouse_rate(g, 100);
	if (rc == 0)
		rc = mouse_rate(g, 80);
	if (rc == 0)
		rc = mouse_read_id(g, id);
	return (rc);
}

static int	mouse_start(t_ps2 *g)
{
	int	rc;

	rc = mouse_rate(g, 100);
	if (rc == 0)
		rc = ps2_dev_cmd(g, PS2_PORT_MOUSE, PS2_D_ENABLE);
	return (rc);
}

int	ps2_mouse_init(t_ps2 *g)
{
	uint8_t	id;
	int		rc;

	rc = ps2_dev_reset(g, PS2_PORT_MOUSE, &id);
	if (rc == 0)
		rc = ps2_dev_cmd(g, PS2_PORT_MOUSE, PS2_D_DEFAULTS);
	if (rc < 0)
	{
		klog_info("input: souris PS/2 absente (%d)", rc);
		return (rc);
	}
	if (mouse_detect_wheel(g, &id) < 0)
		id = 0;
	g->mouse_id = id;
	mousedec_reset(&g->mdec, id == 3);
	rc = mouse_start(g);
	if (rc < 0)
		klog_warn("input: souris PS/2 sans activation (%d)", rc);
	else
		klog_info("input: souris PS/2 prête (id %u)", (unsigned)id);
	return (rc);
}
