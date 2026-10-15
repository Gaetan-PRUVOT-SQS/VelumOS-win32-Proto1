#include "string.h"
#include "vtest.h"

static void	sys_time(void)
{
	int64_t	t0;
	int64_t	t1;

	t0 = v_time_mono();
	vtest_check(t0 > 0, "sys: horloge monotone positive");
	v_sleep(2 * V_NS_PER_MS);
	t1 = v_time_mono();
	vtest_check(t1 - t0 >= (int64_t)(2 * V_NS_PER_MS), "sys: sleep 2 ms");
	vtest_check(v_time_wall() > 0, "sys: heure murale positive");
	vtest_check(v_yield() == 0, "sys: yield");
}

static void	sys_info(void)
{
	t_sysinfo	si;
	t_procinfo	pi;
	uint8_t		a[16];
	uint8_t		b[16];

	vtest_check(v_sysinfo(&si) == 0 && si.abi_version == VELUM_ABI_VERSION,
		"sys: sysinfo, version d'ABI");
	vtest_check(si.ncpus >= 1 && si.mem_total > 0,
		"sys: sysinfo cpus, memoire");
	vtest_check(v_proc_info(0, &pi) == 0 && pi.pid > 0, "sys: proc_info(0)");
	vtest_check(v_getrandom(a, 16, 0) == 16 && v_getrandom(b, 16, 0) == 16
		&& memcmp(a, b, 16) != 0, "sys: getrandom, deux tirages distincts");
	vtest_check(v_getrandom(a, V_RANDOM_MAX + 1, 0) < 0,
		"sys: getrandom refuse plus de 256 octets");
}

static void	sys_vmem(void)
{
	int64_t		va;
	t_vquery	q;
	uint8_t		*p;

	va = v_valloc(0, 8192, PROT_R | PROT_W);
	vtest_check(va > 0, "sys: valloc 2 pages");
	p = (uint8_t *)(uintptr_t)va;
	vtest_check(p[0] == 0 && p[8191] == 0, "sys: pages a zero");
	vtest_check(v_vquery((uint64_t)va, &q) == 0 && q.prot == (PROT_R | PROT_W),
		"sys: vquery rend les droits");
	vtest_check(v_vprotect((uint64_t)va, 4096, PROT_R) == 0,
		"sys: vprotect R");
	vtest_check(v_vprotect((uint64_t)va, 4096, PROT_W | PROT_X) < 0,
		"sys: vprotect W|X refuse");
	vtest_check(v_valloc(0, 4096, PROT_W | PROT_X) < 0,
		"sys: valloc W|X refuse");
	vtest_check(v_valloc(0, 100, PROT_R) > 0,
		"sys: valloc arrondit la longueur");
	vtest_check(v_vfree((uint64_t)va, 8192) == 0, "sys: vfree");
}

static void	sys_objects(void)
{
	int64_t	ev;
	int64_t	tm;

	ev = v_event_create(1, 0);
	vtest_check(ev > 0, "sys: event_create");
	vtest_check(v_wait((t_handle)ev, V_NS_PER_MS) == E_TIMEOUT,
		"sys: wait expire sur evenement non signale");
	vtest_check(v_event_op((t_handle)ev, EV_SET) == 0
		&& v_wait((t_handle)ev, 0) == 0 && v_wait((t_handle)ev, 0) == 0,
		"sys: evenement manuel reste signale");
	vtest_check(v_event_op((t_handle)ev, EV_RESET) == 0
		&& v_wait((t_handle)ev, 0) == E_TIMEOUT, "sys: reset");
	vtest_check(v_close((t_handle)ev) == 0 && v_close((t_handle)ev) < 0,
		"sys: close puis double close refuse");
	tm = v_timer_create(1);
	vtest_check(tm > 0 && v_timer_set((t_handle)tm,
			(uint64_t)v_time_mono() + 5 * V_NS_PER_MS, 0) == 0
		&& v_wait((t_handle)tm, V_NS_PER_SEC) == 0, "sys: minuterie 5 ms");
	v_close((t_handle)tm);
}

void	vtest_sys(void)
{
	sys_time();
	sys_info();
	sys_vmem();
	sys_objects();
}
