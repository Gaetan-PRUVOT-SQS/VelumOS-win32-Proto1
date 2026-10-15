#include "ps2.h"
#include "velum/err.h"

int	ps2_dev_send(t_ps2 *g, int port, uint8_t b)
{
	int	rc;

	if (port == PS2_PORT_MOUSE)
	{
		rc = ps2_ctl_write(g->ops, PS2_C_WRITE2);
		if (rc < 0)
			return (rc);
	}
	return (ps2_data_write(g->ops, b));
}

int	ps2_wait_ack(t_ps2 *g)
{
	uint8_t	b;
	int		i;
	int		rc;

	i = 0;
	while (i < PS2_ACK_SCAN)
	{
		rc = ps2_data_read(g->ops, &b, PS2_T_DEV);
		if (rc < 0)
			return (rc);
		if (b == PS2_R_ACK)
			return (0);
		if (b == PS2_R_RESEND)
			return (E_AGAIN);
		i++;
	}
	return (E_PROTO);
}

int	ps2_dev_cmd(t_ps2 *g, int port, uint8_t cmd)
{
	int	rc;
	int	tries;

	tries = 0;
	rc = E_AGAIN;
	while (rc == E_AGAIN && tries < PS2_RETRY)
	{
		rc = ps2_dev_send(g, port, cmd);
		if (rc == 0)
			rc = ps2_wait_ack(g);
		tries++;
	}
	return (rc);
}

int	ps2_dev_cmd_arg(t_ps2 *g, int port, uint8_t cmd, uint8_t arg)
{
	int	rc;

	rc = ps2_dev_cmd(g, port, cmd);
	if (rc < 0)
		return (rc);
	return (ps2_dev_cmd(g, port, arg));
}

int	ps2_dev_reset(t_ps2 *g, int port, uint8_t *id)
{
	uint8_t	b;
	int		rc;

	*id = 0;
	rc = ps2_dev_cmd(g, port, PS2_D_RESET);
	if (rc < 0)
		return (rc);
	rc = ps2_data_read(g->ops, &b, PS2_T_BAT);
	if (rc < 0)
		return (rc);
	if (b != PS2_R_BAT_OK)
		return (E_IO);
	if (port == PS2_PORT_KBD)
		return (0);
	return (ps2_data_read(g->ops, id, PS2_T_DEV));
}
