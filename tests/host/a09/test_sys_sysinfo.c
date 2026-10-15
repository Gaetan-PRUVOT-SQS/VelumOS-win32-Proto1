#include <string.h>
#include "a09_test.h"
#include "fake.h"
#include "harness.h"
#include "rng_int.h"
#include "velum/err.h"

static int64_t	call_sysinfo(uint64_t ptr)
{
	t_sysargs	a;

	memset(&a, 0, sizeof(a));
	a.a[0] = ptr;
	return (sys_sysinfo(&a));
}

static void	sysinfo_ecrit_la_structure_complete(void)
{
	t_sysinfo	ref;
	t_sysinfo	got;

	fake_reset();
	g_fake.now_ns = 42;
	g_fake.pmm.total_pages = 10;
	fake_user_map(0x20000, sizeof(got), 0);
	h_eq_i64("succes", call_sysinfo(0x20000), 0);
	memcpy(&got, g_fake.user_mem, sizeof(got));
	sysinfo_collect(&ref);
	h_true(!memcmp(&got, &ref, sizeof(got)), "contenu identique a collect");
	fake_user_unmap();
}

static void	sysinfo_refuse_zone_trop_petite_ou_noyau(void)
{
	fake_reset();
	fake_user_map(0x20000, sizeof(t_sysinfo) - 1, 0);
	h_eq_i64("zone d'un octet trop courte", call_sysinfo(0x20000), E_FAULT);
	h_eq_i64("pointeur noyau", call_sysinfo(0xffff800000001000ull), E_FAULT);
	h_eq_i64("pointeur nul", call_sysinfo(0), E_FAULT);
	fake_user_unmap();
}

int	main(void)
{
	h_begin("a09/sys_sysinfo");
	h_run("sys_sysinfo/structure-complete",
		sysinfo_ecrit_la_structure_complete);
	h_run("sys_sysinfo/zone-trop-courte-noyau-nul",
		sysinfo_refuse_zone_trop_petite_ou_noyau);
	return (h_end());
}
