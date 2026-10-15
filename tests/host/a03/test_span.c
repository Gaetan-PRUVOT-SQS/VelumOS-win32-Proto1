#include <string.h>
#include "harness.h"
#include "fake.h"

static void	span_grandes_pages(void)
{
	t_vmreq		rq;
	t_ptlook	look;

	fake_reset();
	g_vmm.ready = false;
	rq.va = HHDM_DEFAULT + 0x40100000;
	rq.pa = 0x40100000;
	rq.len = 5 * 0x100000;
	h_eq_i64("span", vmm_map_span(&g_vmm.kas, &rq, PTE_P | PTE_W | PTE_NX), 0);
	h_true(vmm_lookup(&g_vmm.kas, rq.va, &look), "debut");
	h_eq_u64("debut en 4 Kio", look.size, 4096);
	h_true(vmm_lookup(&g_vmm.kas, HHDM_DEFAULT + 0x40200000, &look), "2M");
	h_eq_u64("bloc aligne en 2 Mio", look.size, PAGE_2M);
	h_true(vmm_lookup(&g_vmm.kas, HHDM_DEFAULT + 0x405ff000, &look), "fin");
	h_eq_u64("dernier bloc 2 Mio", look.size, PAGE_2M);
	h_eq_u64("pa fin", look.pa, 0x405ff000);
	h_true(!vmm_lookup(&g_vmm.kas, HHDM_DEFAULT + 0x40600000, &look), "apres");
	rq.va = HHDM_DEFAULT + 0x40180000;
	rq.pa = 0x40180000;
	rq.len = 0x1000;
	h_eq_i64("deja mappe", vmm_map_span(&g_vmm.kas, &rq, PTE_P), 0);
	vmm_lookup(&g_vmm.kas, rq.va, &look);
	h_true((look.pte & PTE_W) != 0, "le premier gagne");
	g_vmm.ready = true;
	fake_clean("faux pmm propre");
}

static void	add(t_bootinfo *bi, uint64_t base, uint64_t len, uint32_t type)
{
	bi->ranges[bi->nranges].base = base;
	bi->ranges[bi->nranges].length = len;
	bi->ranges[bi->nranges].type = type;
	bi->nranges++;
}

static void	hhdm_construction(void)
{
	static t_bootinfo	bi;

	fake_reset();
	add(&bi, 0x100000, 0x7f00000, MEM_USABLE);
	add(&bi, 0x1000, 0x9e000, MEM_USABLE);
	add(&bi, 0x8000000, 0x100000, MEM_BOOT_RECLAIM);
	add(&bi, 0x9000123, 0x100, MEM_ACPI_RECLAIM);
	add(&bi, 0xa0000000, 0x300000, MEM_FRAMEBUFFER);
	add(&bi, 0xfd000000, 0x100000, MEM_RESERVED);
	add(&bi, 0x200000, 0x100000, MEM_KERNEL);
	add(&bi, 0xfffffffffffff000ull, 0x2000, MEM_USABLE);
	add(&bi, 0x100000000000ull, 0x0000400000000000ull, MEM_USABLE);
	h_eq_i64("plages", vmm_hhdm_build(&bi), 6);
	h_eq_u64("bios", g_vmm.hspan[1].base, BIOS_AREA_LO);
	h_eq_u64("bios RO", g_vmm.hspan[1].bits & PTE_W, 0);
	h_eq_u64("fusion RAM et reclaim", g_vmm.hspan[2].end, 0x8100000);
	h_eq_u64("chevauchement : le premier gagne", g_vmm.hspan[2].base,
		0x100000);
	h_eq_u64("acpi arrondi", g_vmm.hspan[3].end - g_vmm.hspan[3].base, 4096);
	h_eq_u64("fb WC", g_vmm.hspan[4].bits & PTE_PAT4K, PTE_PAT4K);
	h_eq_u64("plafond 64 Tio", vmm_hhdm_top(), HHDM_MAX);
	bi.efi = 1;
	bi.nranges = 500;
	h_eq_i64("uefi sans zone bios, 128 plages", vmm_hhdm_build(&bi), 5);
	fake_clean("faux pmm propre");
}

static void	hhdm_couverture(void)
{
	static t_bootinfo	bi;

	fake_reset();
	add(&bi, 0x1000, 0x9e000, MEM_USABLE);
	add(&bi, 0x100000, 0x100000, MEM_USABLE);
	bi.efi = 1;
	vmm_hhdm_build(&bi);
	h_true(vmm_hhdm_covers(0x1000, 4096), "debut");
	h_true(vmm_hhdm_covers(0x1fffff, 1), "dernier octet");
	h_true(!vmm_hhdm_covers(0x200000, 1), "apres");
	h_true(!vmm_hhdm_covers(0x9f000, 1), "trou");
	h_true(!vmm_hhdm_covers(0x9e000, 0x2000), "a cheval");
	h_true(!vmm_hhdm_covers(0xfffffffffffffff0ull, 0x20), "deborde");
	g_vmm.ready = false;
	h_eq_i64("map hhdm", vmm_hhdm_map(&g_vmm.kas, HHDM_DEFAULT), 0);
	h_true(fake_pte(&g_vmm.kas, HHDM_DEFAULT + 0x150000) != 0, "mappee");
	h_true(!fake_pte(&g_vmm.kas, HHDM_DEFAULT + 0x9f000), "trou absent");
	g_vmm.ready = true;
	fake_clean("faux pmm propre");
}

int	main(void)
{
	h_begin("a03/span");
	h_run("span_grandes_pages", span_grandes_pages);
	h_run("hhdm_construction", hhdm_construction);
	h_run("hhdm_couverture", hhdm_couverture);
	return (h_end());
}
