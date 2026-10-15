#include "fake_check.h"
#include "fake_sys.h"
#include "harness.h"
#include "velum/velum.h"

static int64_t	g_ret;

static void	time_calls(void)
{
	fake_reset();
	g_fsys.ret = g_ret;
	h_eq_i64("yield", v_yield(), g_ret);
	sys_is(SYS_YIELD, 0, 0, 0);
	h_eq_i64("sleep", v_sleep(0x1111), g_ret);
	sys_is(SYS_SLEEP, 0x1111, 0, 0);
	h_eq_i64("time_mono", v_time_mono(), g_ret);
	sys_is(SYS_TIME_MONO, 0, 0, 0);
	h_eq_i64("time_wall", v_time_wall(), g_ret);
	sys_is(SYS_TIME_WALL, 0, 0, 0);
	h_eq_i64("time_set_wall", v_time_set_wall(0x2222), g_ret);
	sys_is(SYS_TIME_SET_WALL, 0x2222, 0, 0);
	h_eq_i64("power", v_power(POWER_REBOOT), g_ret);
	sys_is(SYS_POWER, POWER_REBOOT, 0, 0);
	sys_is_hi(0, 0, 0);
}

static void	display_calls(void)
{
	t_dispinfo	info;
	t_dispmode	modes[3];

	fake_reset();
	g_fsys.ret = g_ret;
	h_eq_i64("display_info", v_display_info(&info), g_ret);
	sys_is(SYS_DISPLAY_INFO, sys_u(&info), 0, 0);
	h_eq_i64("display_map", v_display_map(0x7000), g_ret);
	sys_is(SYS_DISPLAY_MAP, 0x7000, 0, 0);
	h_eq_i64("display_set_mode", v_display_set_mode(1024, 768, 32), g_ret);
	sys_is(SYS_DISPLAY_SET_MODE, 1024, 768, 32);
	h_eq_i64("display_modes", v_display_modes(modes, 3), g_ret);
	sys_is(SYS_DISPLAY_MODES, sys_u(modes), 3, 0);
	h_eq_i64("kcon", v_kcon(1), g_ret);
	sys_is(SYS_KCON, 1, 0, 0);
}

static void	misc_partitions(void)
{
	fake_reset();
	g_fsys.ret = 1700000000ll * 1000000000ll;
	h_eq_i64("time_wall 64 bits", v_time_wall(), g_fsys.ret);
	g_fsys.ret = 1ll << 40;
	h_eq_i64("time_mono 64 bits", v_time_mono(), 1ll << 40);
	g_fsys.ret = 0x7fff00001000ll;
	h_eq_i64("display_map adresse haute", v_display_map(0), g_fsys.ret);
	v_kcon(5);
	sys_is(SYS_KCON, 1, 0, 0);
	v_kcon(-1);
	sys_is(SYS_KCON, 1, 0, 0);
	v_kcon(0);
	sys_is(SYS_KCON, 0, 0, 0);
	v_sleep(UINT64_MAX);
	sys_is(SYS_SLEEP, UINT64_MAX, 0, 0);
}

int	main(void)
{
	h_begin("a14/sys_misc");
	g_ret = 13;
	h_run("vtime/exigence : temps et alimentation, retour positif",
		time_calls);
	h_run("vdisplay/exigence : affichage, retour positif", display_calls);
	g_ret = E_PERM;
	h_run("vtime/exigence : temps et alimentation, retour negatif",
		time_calls);
	h_run("vdisplay/exigence : affichage, retour negatif", display_calls);
	h_run("vtime/valeur limite : 64 bits, booleens kcon, sleep maximal",
		misc_partitions);
	return (h_end());
}
