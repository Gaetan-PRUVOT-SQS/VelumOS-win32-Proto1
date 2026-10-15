#include "fake_check.h"
#include "fake_sys.h"
#include "harness.h"
#include "velum/velum.h"

static int64_t	g_ret;

static void	input_calls(void)
{
	t_inpevent	ev[2];
	char		name[8];

	fake_reset();
	g_fsys.ret = g_ret;
	h_eq_i64("input_open", v_input_open(0x91), g_ret);
	sys_is(SYS_INPUT_OPEN, 0x91, 0, 0);
	h_eq_i64("input_read", v_input_read(0x92, ev, 2), g_ret);
	sys_is(SYS_INPUT_READ, 0x92, sys_u(ev), 2);
	h_eq_i64("input_layout", v_input_layout(V_LAYOUT_SET, name, 5), g_ret);
	sys_is(SYS_INPUT_LAYOUT, V_LAYOUT_SET, sys_u(name), 5);
	h_eq_i64("input_leds", v_input_leds(0x3), g_ret);
	sys_is(SYS_INPUT_LEDS, 0x3, 0, 0);
	h_eq_i64("input_mouse_cfg", v_input_mouse_cfg(7, 1), g_ret);
	sys_is(SYS_INPUT_MOUSE_CFG, 7, 1, 0);
	sys_is_hi(0, 0, 0);
}

static void	mem_calls(void)
{
	t_vquery	q;

	fake_reset();
	g_fsys.ret = g_ret;
	h_eq_i64("valloc", v_valloc(0x10000, 0x3000, PROT_R | PROT_W), g_ret);
	sys_is(SYS_VALLOC, 0x10000, 0x3000, PROT_R | PROT_W);
	sys_is_hi(0, 0, 0);
	h_eq_i64("vfree", v_vfree(0x20000, 0x4000), g_ret);
	sys_is(SYS_VFREE, 0x20000, 0x4000, 0);
	h_eq_i64("vprotect", v_vprotect(0x30000, 0x5000, PROT_R | PROT_X), g_ret);
	sys_is(SYS_VPROTECT, 0x30000, 0x5000, PROT_R | PROT_X);
	h_eq_i64("vquery", v_vquery(0x40000, &q), g_ret);
	sys_is(SYS_VQUERY, 0x40000, sys_u(&q), 0);
}

static void	inpmem_partitions(void)
{
	char	name[4];

	fake_reset();
	g_fsys.ret = 0x7ffff0001000ll;
	h_eq_i64("valloc adresse haute", v_valloc(0, 4096, PROT_R), g_fsys.ret);
	g_fsys.ret = E_NOMEM;
	h_eq_i64("valloc epuisement", v_valloc(0, 4096, PROT_R), E_NOMEM);
	h_eq_i64("input_layout lecture", v_input_layout(V_LAYOUT_GET, name, 4),
		E_NOMEM);
	sys_is(SYS_INPUT_LAYOUT, V_LAYOUT_GET, sys_u(name), 4);
	h_eq_i64("input_layout pointeur nul transmis",
		v_input_layout(V_LAYOUT_GET, NULL, 0), E_NOMEM);
	sys_is(SYS_INPUT_LAYOUT, V_LAYOUT_GET, 0, 0);
	h_eq_i64("vquery pointeur nul transmis", v_vquery(8, NULL), E_NOMEM);
	sys_is(SYS_VQUERY, 8, 0, 0);
}

int	main(void)
{
	h_begin("a14/sys_inpmem");
	g_ret = 3;
	h_run("vinput/exigence : entrees, retour positif", input_calls);
	h_run("vmem/exigence : memoire virtuelle, retour positif", mem_calls);
	g_ret = E_INVAL;
	h_run("vinput/exigence : entrees, retour negatif", input_calls);
	h_run("vmem/exigence : memoire virtuelle, retour negatif", mem_calls);
	h_run("vmem/partition : adresse haute, epuisement, pointeurs nuls",
		inpmem_partitions);
	return (h_end());
}
