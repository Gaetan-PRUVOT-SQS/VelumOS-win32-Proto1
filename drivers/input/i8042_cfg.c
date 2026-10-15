#include "ps2.h"
#include "velum/err.h"

int	ps2_ctl_query(const t_ps2ops *o, uint8_t cmd, uint8_t *resp)
{
	int	rc;

	rc = ps2_ctl_write(o, cmd);
	if (rc < 0)
		return (rc);
	return (ps2_data_read(o, resp, PS2_T_CTL));
}

int	i8042_cfg_read(t_ps2 *g, uint8_t *cfg)
{
	return (ps2_ctl_query(g->ops, PS2_C_READ_CFG, cfg));
}

int	i8042_cfg_write(t_ps2 *g, uint8_t cfg)
{
	int	rc;

	rc = ps2_ctl_write(g->ops, PS2_C_WRITE_CFG);
	if (rc < 0)
		return (rc);
	return (ps2_data_write(g->ops, cfg));
}

int	i8042_selftest(t_ps2 *g)
{
	uint8_t	r;
	int		rc;

	rc = ps2_ctl_query(g->ops, PS2_C_SELFTEST, &r);
	if (rc < 0)
		return (rc);
	if (r != PS2_SELFTEST_OK)
		return (E_IO);
	return (0);
}

int	i8042_port_test(t_ps2 *g, uint8_t cmd)
{
	uint8_t	r;
	int		rc;

	rc = ps2_ctl_query(g->ops, cmd, &r);
	if (rc < 0)
		return (rc);
	if (r != PS2_PORTTEST_OK)
		return (E_IO);
	return (0);
}
