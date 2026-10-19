#include <stdint.h>
#include <string.h>
#include "../harness.h"
#include "meminfo.h"

static void	construction_nominale(void)
{
	t_pmm_stats	ps;
	t_meminfo	mi;

	memset(&ps, 0, sizeof(ps));
	ps.total_pages = 4194304;
	ps.free_pages = 4000000;
	ps.reserved_pages = 262144;
	ps.owned[PMM_KERNEL] = 100;
	ps.owned[PMM_FB] = 7;
	ps.alloc_calls = 11;
	ps.free_calls = 5;
	ps.fail_calls = 2;
	meminfo_build(&mi, &ps, 1792ull << 20, 12345);
	h_eq_u64("version", mi.version, 1);
	h_eq_u64("taille de page", mi.page_size, 4096);
	h_eq_u64("étendue 17 Gio", mi.span_bytes, 17ull << 30);
	h_eq_u64("utilisable 16 Gio", mi.usable_bytes, 16ull << 30);
	h_eq_u64("libre", mi.free_bytes, 4000000ull * 4096);
	h_eq_u64("réservé 1 Gio", mi.reserved_bytes, 1ull << 30);
	h_eq_u64("fenêtre du tas", mi.heap_window_bytes, 1792ull << 20);
	h_eq_u64("tas vivant", mi.heap_live_bytes, 12345);
	h_eq_u64("pages noyau", mi.owned_pages[PMM_KERNEL], 100);
	h_eq_u64("pages écran", mi.owned_pages[PMM_FB], 7);
	h_eq_u64("allocations", mi.alloc_calls, 11);
	h_eq_u64("libérations", mi.free_calls, 5);
	h_eq_u64("échecs", mi.fail_calls, 2);
}

static void	construction_64_gio_sans_debordement(void)
{
	t_pmm_stats	ps;
	t_meminfo	mi;

	memset(&ps, 0, sizeof(ps));
	ps.total_pages = 16ull << 20;
	ps.free_pages = 16ull << 20;
	meminfo_build(&mi, &ps, 0, 0);
	h_eq_u64("64 Gio exacts", mi.usable_bytes, 64ull << 30);
	h_eq_u64("libre 64 Gio", mi.free_bytes, 64ull << 30);
}

static void	construction_vide(void)
{
	t_pmm_stats	ps;
	t_meminfo	mi;

	memset(&ps, 0, sizeof(ps));
	memset(&mi, 0xaa, sizeof(mi));
	meminfo_build(&mi, &ps, 0, 0);
	h_eq_u64("version", mi.version, 1);
	h_eq_u64("tout à zéro", mi.span_bytes + mi.usable_bytes + mi.free_bytes
		+ mi.reserved_bytes + mi.heap_window_bytes + mi.heap_live_bytes, 0);
	h_eq_u64("propriétaires remis à zéro", mi.owned_pages[PMM_OWNERS - 1], 0);
}

int	main(void)
{
	h_begin("e10/meminfo");
	h_run("construction nominale 16 Gio", construction_nominale);
	h_run("construction 64 Gio", construction_64_gio_sans_debordement);
	h_run("construction vide", construction_vide);
	return (h_end());
}
