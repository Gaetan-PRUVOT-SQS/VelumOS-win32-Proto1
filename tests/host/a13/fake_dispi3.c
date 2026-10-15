#include "fakes.h"

bool	fd_wrote(uint32_t i, uint16_t idx, uint16_t val)
{
	return (g_fd.log[i][0] == idx && g_fd.log[i][1] == val);
}

uint64_t	fx_u(const void *p)
{
	g_fp.ok_base = (uint64_t)(uintptr_t)p;
	g_fp.ok_len = 4096;
	return ((uint64_t)(uintptr_t)p);
}

int64_t	fx_map(uint64_t hint)
{
	return (fake_sys(SYS_DISPLAY_MAP, hint, 0, 0));
}
