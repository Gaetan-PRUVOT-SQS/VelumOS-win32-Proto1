#include "d08.h"

t_d08fail	g_d08;

void	*__wrap_calloc(size_t n, size_t size)
{
	g_d08.calls++;
	if (g_d08.armed && g_d08.left == 0)
		return (NULL);
	if (g_d08.armed)
		g_d08.left--;
	return (__real_calloc(n, size));
}

void	d08_fail_after(uint32_t n)
{
	g_d08.armed = 1;
	g_d08.left = n;
}

void	d08_fail_off(void)
{
	g_d08.armed = 0;
}
