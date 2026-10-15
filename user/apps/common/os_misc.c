#include "velum/velum.h"
#include "platform.h"

int	os_power(uint32_t op)
{
	return (v_power(op));
}

int	os_sysinfo(t_sysinfo *out)
{
	return (v_sysinfo(out));
}

void	os_log(const char *msg)
{
	v_log(V_LOG_INFO, msg);
}

int	os_wait1(t_handle h, uint64_t timeout_ns)
{
	return (v_wait(h, timeout_ns));
}

int	os_read_file(const char *path, char *buf, size_t max, size_t *len)
{
	int64_t	h;
	int64_t	n;
	size_t	total;

	h = v_open(path, O_RDONLY, 0);
	if (h < 0)
		return ((int)h);
	total = 0;
	n = 1;
	while (n > 0 && total < max)
	{
		n = v_read((t_handle)h, buf + total, max - total);
		if (n > 0)
			total += (size_t)n;
	}
	v_close((t_handle)h);
	if (n < 0)
		return ((int)n);
	*len = total;
	return (0);
}
