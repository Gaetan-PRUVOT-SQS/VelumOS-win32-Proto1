#include <string.h>
#include "fake.h"
#include "velum/err.h"
#include "velum/vmm.h"

t_fuser	g_fuser;

void	fuser_reset(void)
{
	memset(g_fuser.mem, 0, sizeof(g_fuser.mem));
	g_fuser.fault_addr = 0;
	g_fuser.lock_violations = 0;
	g_fuser.copies = 0;
}

bool	user_range_ok(t_uptr addr, size_t n)
{
	if (addr < FU_BASE || addr > FU_BASE + FU_SIZE)
		return (false);
	return (n <= FU_BASE + FU_SIZE - addr);
}

static int	fuser_check(t_uptr addr, size_t n)
{
	if (g_fsync.held != 0)
		g_fuser.lock_violations++;
	g_fuser.copies++;
	if (!user_range_ok(addr, n))
		return (E_FAULT);
	if (g_fuser.fault_addr && g_fuser.fault_addr >= addr
		&& g_fuser.fault_addr < addr + n)
		return (E_FAULT);
	return (0);
}

int	copy_to_user(t_uptr dst, const void *src, size_t n)
{
	int	rc;

	rc = fuser_check(dst, n);
	if (rc < 0)
		return (rc);
	memcpy(&g_fuser.mem[dst - FU_BASE], src, n);
	return (0);
}

int	copy_from_user(void *dst, t_uptr src, size_t n)
{
	int	rc;

	rc = fuser_check(src, n);
	if (rc < 0)
		return (rc);
	memcpy(dst, &g_fuser.mem[src - FU_BASE], n);
	return (0);
}
