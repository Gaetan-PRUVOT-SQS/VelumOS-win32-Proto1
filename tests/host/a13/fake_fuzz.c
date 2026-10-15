#include "kfix.h"
#include "velum/err.h"
#include "velum/vmm.h"

bool	fx_rc_ok(int64_t rc)
{
	if (rc >= 0)
		return (true);
	return (rc == E_PERM || rc == E_FAULT || rc == E_INVAL || rc == E_NODEV
		|| rc == E_NOMEM || rc == E_NOTSUP || rc == E_IO || rc == E_NOSPC);
}

static uint64_t	pick_arg(uint64_t *seed, void *buf)
{
	uint64_t	r;

	r = kfix_rnd(seed);
	if ((r & 7) == 0)
		return (0);
	if ((r & 7) == 1)
		return (FAKE_BAD_PTR);
	if ((r & 7) == 2)
		return (fx_u(buf));
	if ((r & 7) == 3)
		return (r >> 8);
	if ((r & 7) == 4)
		return ((r >> 8) % 4096);
	if ((r & 7) == 5)
		return (800 + ((r >> 8) % 1200));
	if ((r & 7) == 6)
		return (UINT64_MAX - ((r >> 8) % 4));
	return (USER_TOP - 4096 * ((r >> 8) % 1024));
}

int	fx_fuzz_one(uint64_t *seed, uint8_t *buf)
{
	uint64_t	a[3];
	uint32_t	num;
	int64_t		rc;

	num = 0x60 + (uint32_t)(kfix_rnd(seed) % 6);
	a[0] = pick_arg(seed, buf);
	a[1] = pick_arg(seed, buf);
	a[2] = pick_arg(seed, buf);
	fake_proc_set(10 + (uint32_t)(kfix_rnd(seed) % 3),
		(uint32_t)(kfix_rnd(seed) % 128));
	if (kfix_rnd(seed) % 50 == 0)
		g_fp.cur = NULL;
	rc = fake_sys(num, a[0], a[1], a[2]);
	if (num == SYS_DISPLAY_MAP && rc >= 0 && rc % 4096)
		return (1);
	if (num == SYS_DISPLAY_MAP && rc >= 0 && rc < (int64_t)USER_MIN)
		return (1);
	return (!fx_rc_ok(rc) && num != 0x65);
}

uint32_t	fx_canary(const uint8_t *p, size_t n)
{
	size_t		i;
	uint32_t	bad;

	i = 0;
	bad = 0;
	while (i < n)
	{
		bad += (p[i] != 0xaa);
		i++;
	}
	return (bad);
}
