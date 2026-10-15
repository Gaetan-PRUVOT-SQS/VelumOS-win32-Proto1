#include "fake.h"
#include "rng_int.h"
#include "velum/err.h"

static uint64_t	fake_now(void)
{
	return (g_fake.now_ns);
}

static uint64_t	fake_cycles(void)
{
	g_fake.cycles_calls++;
	g_fake.rng_state ^= g_fake.rng_state << 13;
	g_fake.rng_state ^= g_fake.rng_state >> 7;
	g_fake.rng_state ^= g_fake.rng_state << 17;
	g_fake.tsc += 1 + (g_fake.rng_state & 0xff);
	return (g_fake.tsc);
}

static int	fake_hw(t_fake_hw *hw, uint64_t *out)
{
	hw->calls++;
	if (hw->mode == HW_ABSENT)
		return (E_NODEV);
	if (hw->mode == HW_BROKEN)
		return (E_AGAIN);
	if (hw->mode == HW_FLAKY && hw->pending > 0)
	{
		hw->pending--;
		return (E_AGAIN);
	}
	*out = fake_cycles() * 0x9e3779b97f4a7c15ull;
	hw->pending = hw->flaky_fails;
	return (0);
}

static int	fake_rdseed(uint64_t *out)
{
	return (fake_hw(&g_fake.rdseed, out));
}

static int	fake_rdrand(uint64_t *out)
{
	return (fake_hw(&g_fake.rdrand, out));
}

const t_rng_env	g_rng_env = {fake_now, fake_cycles, fake_rdseed, fake_rdrand};
