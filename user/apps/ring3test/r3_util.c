#include "r3.h"

int64_t	r3_sys(uint64_t num, uint64_t a0, uint64_t a1, uint64_t a2)
{
	uint64_t	a[V_SYS_ARGS];

	memset(a, 0, sizeof(a));
	a[0] = a0;
	a[1] = a1;
	a[2] = a2;
	return (v_syscall6(num, a));
}

void	r3_check(t_r3 *t, const char *name, int64_t got, int64_t want)
{
	if (got == want)
	{
		t->ok++;
		v_logf(V_LOG_INFO, "[ok] %s", name);
		return ;
	}
	t->ko++;
	v_logf(V_LOG_ERR, "[ko] %s : obtenu %lld, attendu %lld", name,
		(long long)got, (long long)want);
}

void	r3_skip(t_r3 *t, const char *name)
{
	t->skip++;
	v_logf(V_LOG_WARN, "[--] %s", name);
}

int64_t	r3_spawn_raw(uint64_t path, uint64_t len, uint64_t args,
		uint64_t alen)
{
	uint64_t	a[V_SYS_ARGS];

	memset(a, 0, sizeof(a));
	a[0] = path;
	a[1] = len;
	a[2] = args;
	a[3] = alen;
	return (v_syscall6(SYS_PROC_SPAWN, a));
}

int64_t	r3_reap(int64_t h, t_procinfo *info)
{
	int64_t	rc;

	memset(info, 0, sizeof(*info));
	rc = v_wait((t_handle)h, R3_WAIT_NS);
	if (rc == 0)
		rc = v_proc_info((t_handle)h, info);
	v_close((t_handle)h);
	return (rc);
}
