#include "display_int.h"
#include "kfix.h"
#include "velum/err.h"
#include "velum/vmm.h"

static void	two_processes(void)
{
	int64_t	a;
	int64_t	b;

	fx_display(1024, 768);
	a = fx_map(0);
	fake_proc_set(11, PF_DISPLAY);
	b = fx_map(0);
	h_true(a > 0 && b > 0, "deux adresses");
	h_eq_i64("deux mappages", fake_vmm_live_maps(), 2);
	h_eq_i64("le second retrouve le sien", fx_map(0), b);
	fake_proc_set(10, PF_DISPLAY);
	h_eq_i64("le premier retrouve le sien", fx_map(0), a);
	h_eq_u64("deux vmm_map seulement", g_fvmm.maps, 2);
}

static void	slot_eviction(void)
{
	uint32_t	pid;

	fx_display(1024, 768);
	pid = 10;
	while (pid < 15)
	{
		fake_proc_set(pid, PF_DISPLAY);
		h_true(fx_map(0) > 0, "mappage de chaque processus");
		pid++;
	}
	h_eq_i64("cinq mappages vivants", fake_vmm_live_maps(), 5);
	fake_proc_set(12, PF_DISPLAY);
	h_true(fx_map(0) > 0, "processus evince : nouveau mappage");
	h_eq_i64("aucun plantage et 6 mappages", fake_vmm_live_maps(), 6);
}

static void	memory_type_follows_cache(void)
{
	fx_display(1024, 768);
	g_fk.cpu.pat = false;
	display_boot_init();
	h_true(fx_map(0) > 0, "mappage sans PAT");
	h_eq_u64("sans cache comme le noyau", fx_live_map(0)->flags,
		VM_NOCACHE | VM_USER | VM_R | VM_W);
	h_eq_u64("noyau", g_display.map_flags, VM_NOCACHE);
}

int	main(void)
{
	h_begin("a13/sys_mapproc");
	h_run("mapproc deux processus", two_processes);
	h_run("mapproc eviction d'emplacement", slot_eviction);
	h_run("mapproc type de cache identique", memory_type_follows_cache);
	return (h_end());
}
