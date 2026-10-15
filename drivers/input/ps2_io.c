#include "ps2.h"
#include "velum/err.h"
#include "velum/timer.h"

int	ps2_wait(const t_ps2ops *o, uint8_t mask, uint8_t want, uint64_t timeout_ns)
{
	uint64_t	deadline;

	deadline = time_now_ns() + timeout_ns;
	while (time_now_ns() < deadline)
	{
		if ((o->status() & mask) == want)
			return (0);
	}
	if ((o->status() & mask) == want)
		return (0);
	return (E_TIMEOUT);
}

int	ps2_ctl_write(const t_ps2ops *o, uint8_t cmd)
{
	int	rc;

	rc = ps2_wait(o, PS2_ST_IBF, 0, PS2_T_CTL);
	if (rc < 0)
		return (rc);
	o->write_cmd(cmd);
	return (0);
}

int	ps2_data_write(const t_ps2ops *o, uint8_t v)
{
	int	rc;

	rc = ps2_wait(o, PS2_ST_IBF, 0, PS2_T_CTL);
	if (rc < 0)
		return (rc);
	o->write_data(v);
	return (0);
}

int	ps2_data_read(const t_ps2ops *o, uint8_t *v, uint64_t timeout_ns)
{
	uint8_t	st;
	int		rc;

	rc = ps2_wait(o, PS2_ST_OBF, PS2_ST_OBF, timeout_ns);
	if (rc < 0)
		return (rc);
	st = o->status();
	*v = o->read();
	if (st & PS2_ST_ERR)
		return (E_IO);
	return (0);
}

void	ps2_flush(const t_ps2ops *o)
{
	int	n;

	n = 0;
	while (n < PS2_FLUSH_MAX && (o->status() & PS2_ST_OBF))
	{
		o->read();
		n++;
	}
}
