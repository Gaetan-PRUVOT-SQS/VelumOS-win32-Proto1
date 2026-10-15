#include "r3.h"

int64_t	r3_child(const char *mode, uint32_t flags)
{
	return (v_spawn(R3_SELF, mode, strlen(mode) + 1, flags));
}

static void	r3_expect_child(t_r3 *t, const char *mode, uint32_t flags,
				int64_t code)
{
	t_procinfo	info;
	int64_t		h;
	int64_t		rc;

	h = r3_child(mode, flags);
	r3_check(t, mode, h > 0, 1);
	if (h <= 0)
		return ;
	rc = r3_reap(h, &info);
	r3_check(t, "enfant attendu et lu", rc, 0);
	r3_check(t, "enfant zombie", info.state, PS_ZOMBIE);
	r3_check(t, "code de sortie de l'enfant", (int32_t)info.reserved, code);
	r3_check(t, "privilèges de l'enfant", info.flags, flags);
}

static void	r3_kill_child(t_r3 *t)
{
	t_procinfo	info;
	int64_t		h;
	int64_t		rc;

	h = r3_child("dort", PF_ALL);
	r3_check(t, "enfant dormeur lancé", h > 0, 1);
	if (h <= 0)
		return ;
	r3_sys(SYS_SLEEP, R3_NAP_NS, 0, 0);
	r3_check(t, "kill de l'enfant", r3_sys(SYS_PROC_KILL, h, 7, 0), 0);
	rc = r3_reap(h, &info);
	r3_check(t, "enfant tué attendu", rc, 0);
	r3_check(t, "code de l'enfant tué", (int32_t)info.reserved, 7);
}

static void	r3_spawn_many(t_r3 *t)
{
	t_procinfo	info;
	int64_t		h;
	int			i;
	int			fails;

	fails = 0;
	i = 0;
	while (i < 100)
	{
		h = r3_child("exit42", 0);
		if (h <= 0 || r3_reap(h, &info) != 0 || info.reserved != 42)
			fails++;
		i++;
	}
	r3_check(t, "100 lancements et fins", fails, 0);
}

void	r3_suite_child(t_r3 *t)
{
	r3_expect_child(t, "null", PF_ALL, -1);
	r3_expect_child(t, "div0", PF_ALL, -1);
	r3_expect_child(t, "exit42", PF_SPAWN, 42);
	r3_expect_child(t, "noperm", 0, 0);
	r3_kill_child(t);
	r3_spawn_many(t);
}
