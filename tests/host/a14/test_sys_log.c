#include "fake_check.h"
#include "fake_sys.h"
#include "harness.h"
#include "velum/velum.h"

static int64_t	g_ret;

static void	log_calls(void)
{
	const char	*msg = "hello";
	char		buf[16];
	t_sysinfo	si;

	fake_reset();
	g_fsys.ret = g_ret;
	h_eq_i64("log", v_log(V_LOG_WARN, msg), g_ret);
	sys_is(SYS_LOG, V_LOG_WARN, sys_u(msg), 5);
	sys_is_hi(0, 0, 0);
	h_eq_i64("getrandom", v_getrandom(buf, 16, 0x5), g_ret);
	sys_is(SYS_GETRANDOM, sys_u(buf), 16, 0x5);
	h_eq_i64("sysinfo", v_sysinfo(&si), g_ret);
	sys_is(SYS_SYSINFO, sys_u(&si), 0, 0);
}

static void	log_partitions(void)
{
	const char	*empty = "";
	const char	*one = "x";
	char		buf[4];

	fake_reset();
	h_eq_i64("log message vide", v_log(V_LOG_INFO, empty), 0);
	sys_is(SYS_LOG, V_LOG_INFO, sys_u(empty), 0);
	v_log(0xffffffffu, one);
	sys_is(SYS_LOG, 0xffffffffu, sys_u(one), 1);
	v_getrandom(buf, V_RANDOM_MAX, 0);
	sys_is(SYS_GETRANDOM, sys_u(buf), V_RANDOM_MAX, 0);
	h_eq_i64("getrandom buffer nul transmis",
		v_getrandom(NULL, 0, 0), 0);
	sys_is(SYS_GETRANDOM, 0, 0, 0);
	h_eq_i64("sysinfo pointeur nul transmis", v_sysinfo(NULL), 0);
	sys_is(SYS_SYSINFO, 0, 0, 0);
}

int	main(void)
{
	h_begin("a14/sys_log");
	g_ret = 4;
	h_run("vmisc/exigence : log, getrandom, sysinfo, retour positif",
		log_calls);
	g_ret = E_FAULT;
	h_run("vmisc/exigence : log, getrandom, sysinfo, retour negatif",
		log_calls);
	h_run("vmisc/partition : message vide, niveau extreme, tailles",
		log_partitions);
	return (h_end());
}
