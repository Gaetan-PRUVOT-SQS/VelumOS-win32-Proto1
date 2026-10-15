#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include "fake.h"

t_fake	g_fake;

void	fake_reset(void)
{
	memset(&g_fake, 0, sizeof(g_fake));
	g_fake.rng_state = 0x9e3779b97f4a7c15ull;
	g_fake.tsc = 1000;
	g_fake.cycles_on = 1;
	g_fake.rdseed.mode = HW_OK;
	g_fake.rdrand.mode = HW_OK;
	g_fake.ncpus = 1;
	g_fake.vec_next = 0x40;
	g_fake.apic = 3;
	snprintf(g_fake.brand, sizeof(g_fake.brand), "%s", "Fake CPU @ 1 GHz");
}

void	fake_log_append(const char *fmt, ...)
{
	va_list	ap;
	size_t	room;

	room = FAKE_LOG_MAX - g_fake.log_len;
	if (room < 2)
		return ;
	va_start(ap, fmt);
	vsnprintf(g_fake.log + g_fake.log_len, room, fmt, ap);
	va_end(ap);
	g_fake.log_len += strlen(g_fake.log + g_fake.log_len);
}

int	fake_log_has(const char *needle)
{
	return (strstr(g_fake.log, needle) != NULL);
}
