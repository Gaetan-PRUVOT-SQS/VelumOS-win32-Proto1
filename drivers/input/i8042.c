#include "ps2.h"
#include "velum/err.h"

static int	i8042_quiesce(t_ps2 *g, uint8_t *cfg)
{
	int	rc;

	rc = ps2_ctl_write(g->ops, PS2_C_DIS1);
	if (rc == 0)
		rc = ps2_ctl_write(g->ops, PS2_C_DIS2);
	if (rc < 0)
		return (rc);
	ps2_flush(g->ops);
	rc = i8042_cfg_read(g, cfg);
	if (rc < 0)
		return (rc);
	*cfg &= ~(PS2_CFG_IRQ1 | PS2_CFG_IRQ2 | PS2_CFG_XLAT);
	return (i8042_cfg_write(g, *cfg));
}

static int	i8042_probe_dual(t_ps2 *g)
{
	uint8_t	cfg;
	int		rc;

	g->dual = 0;
	rc = ps2_ctl_write(g->ops, PS2_C_EN2);
	if (rc < 0)
		return (rc);
	rc = i8042_cfg_read(g, &cfg);
	if (rc < 0)
		return (rc);
	if (!(cfg & PS2_CFG_CLK2_OFF))
		g->dual = 1;
	return (ps2_ctl_write(g->ops, PS2_C_DIS2));
}

static int	i8042_test_ports(t_ps2 *g)
{
	g->kbd_ok = (i8042_port_test(g, PS2_C_TEST1) == 0);
	g->mouse_ok = 0;
	if (g->dual)
		g->mouse_ok = (i8042_port_test(g, PS2_C_TEST2) == 0);
	if (!g->kbd_ok && !g->mouse_ok)
		return (E_NODEV);
	return (0);
}

int	i8042_init(t_ps2 *g)
{
	uint8_t	cfg;
	int		rc;

	if (g->ops->status() == 0xff)
		return (E_NODEV);
	rc = i8042_quiesce(g, &cfg);
	if (rc == 0)
		rc = i8042_selftest(g);
	if (rc == 0)
		rc = i8042_cfg_write(g, cfg);
	if (rc == 0)
		rc = i8042_probe_dual(g);
	if (rc < 0)
		return (rc);
	return (i8042_test_ports(g));
}
