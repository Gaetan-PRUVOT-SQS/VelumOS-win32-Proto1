#include "fake_8042.h"

static const t_ps2ops	g_f8042_ops = {f8042_status, f8042_read,
	f8042_write_cmd, f8042_write_data};

const t_ps2ops	*f8042_ops(void)
{
	return (&g_f8042_ops);
}

const t_ps2ops	*ps2_hw_ops(void)
{
	return (&g_f8042_ops);
}
