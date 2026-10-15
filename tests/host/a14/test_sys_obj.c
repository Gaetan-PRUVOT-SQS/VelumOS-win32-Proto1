#include "fake_check.h"
#include "fake_sys.h"
#include "harness.h"
#include "velum/velum.h"

static int64_t	g_ret;

static void	obj_handles(void)
{
	t_handle	list[2];

	fake_reset();
	g_fsys.ret = g_ret;
	h_eq_i64("close", v_close(0x11), g_ret);
	sys_is(SYS_CLOSE, 0x11, 0, 0);
	sys_is_hi(0, 0, 0);
	h_eq_i64("dup", v_dup(0x12, HR_READ | HR_DUP), g_ret);
	sys_is(SYS_DUP, 0x12, HR_READ | HR_DUP, 0);
	h_eq_i64("wait", v_wait(0x13, 5000), g_ret);
	sys_is(SYS_WAIT, 0x13, 5000, 0);
	h_eq_i64("wait_many", v_wait_many(list, 2, WAIT_ALL, 9000), g_ret);
	sys_is(SYS_WAIT_MANY, sys_u(list), 2, WAIT_ALL);
	sys_is_hi(9000, 0, 0);
	h_eq_i64("event_op", v_event_op(0x14, EV_PULSE), g_ret);
	sys_is(SYS_EVENT_OP, 0x14, EV_PULSE, 0);
}

static void	obj_sections(void)
{
	t_vmaprange	range;

	fake_reset();
	g_fsys.ret = g_ret;
	h_eq_i64("event_create", v_event_create(1, 0), g_ret);
	sys_is(SYS_EVENT_CREATE, 1, 0, 0);
	h_eq_i64("section_create", v_section_create(0x40000, PROT_R), g_ret);
	sys_is(SYS_SECTION_CREATE, 0x40000, PROT_R, 0);
	h_eq_i64("section_map", v_section_map(0x21, 0x1000, 0x2000, PROT_W), g_ret);
	sys_is(SYS_SECTION_MAP, 0x21, 0, PROT_W);
	sys_is_hi(0x1000, 0x2000, 0);
	range.offset = 0x3000;
	range.len = 0x4000;
	range.prot = PROT_R | PROT_W;
	range.reserved = 0;
	h_eq_i64("map_at", v_section_map_at(0x22, 0x7000, &range), g_ret);
	sys_is(SYS_SECTION_MAP, 0x22, 0x7000, PROT_R | PROT_W);
	sys_is_hi(0x3000, 0x4000, 0);
}

static void	obj_partitions(void)
{
	fake_reset();
	h_eq_i64("event_create booleens 5 et -3", v_event_create(5, -3), 0);
	sys_is(SYS_EVENT_CREATE, 1, 1, 0);
	v_event_create(0, 0);
	sys_is(SYS_EVENT_CREATE, 0, 0, 0);
	fake_reset();
	h_eq_i64("map_at NULL", v_section_map_at(1, 0, NULL), E_FAULT);
	sys_none();
	g_fsys.ret = 0x123456789ll;
	h_eq_i64("dup 64 bits", v_dup(1, HR_ALL), 0x123456789ll);
	h_eq_i64("map 64 bits", v_section_map(1, 0, 4096, PROT_R), 0x123456789ll);
}

int	main(void)
{
	h_begin("a14/sys_obj");
	g_ret = 7;
	h_run("vobj/exigence : handles et attente, retour positif", obj_handles);
	h_run("vobj/exigence : sections et evenements, retour positif",
		obj_sections);
	g_ret = E_PERM;
	h_run("vobj/exigence : handles et attente, retour negatif", obj_handles);
	h_run("vobj/exigence : sections et evenements, retour negatif",
		obj_sections);
	h_run("vobj/partition : booleens, pointeur nul, retour 64 bits",
		obj_partitions);
	return (h_end());
}
