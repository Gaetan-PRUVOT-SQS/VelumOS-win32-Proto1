#include "harness.h"
#include "fake.h"

static void	io_map_decalage(void)
{
	uint8_t		*p;
	uint64_t	pte;

	fake_reset();
	p = vmm_io_map(0xfee00020, 0x10, VM_R | VM_W);
	h_true(p != NULL, "io_map");
	h_eq_u64("decalage garde", (uintptr_t)p & 4095, 0x20);
	h_true((uintptr_t)p >= KIO_BASE + 4096, "fenetre io avec garde");
	pte = fake_pte(&g_vmm.kas, (uintptr_t)p);
	h_eq_u64("UC par defaut", vmm_pte_flags(pte, 4096),
		VM_R | VM_W | VM_NOCACHE | VM_GLOBAL);
	h_eq_u64("pa", pte & PTE_ADDR, 0xfee00000);
	h_true(!fake_pte(&g_vmm.kas, (uintptr_t)p - 4096), "garde dessous");
	vmm_io_unmap(p, 0x10);
	h_true(!fake_pte(&g_vmm.kas, (uintptr_t)p), "demappe");
	p = vmm_io_map(0xfd000000, 0x2000, VM_WC);
	h_eq_u64("WC lecture ecriture", vmm_pte_flags(fake_pte(&g_vmm.kas,
				(uintptr_t)p + 4096), 4096), VM_R | VM_W | VM_WC | VM_GLOBAL);
	h_eq_u64("lecture seule demandee", vmm_pte_flags(fake_pte(&g_vmm.kas,
				(uintptr_t)vmm_io_map(0xfed00000, 8, VM_R)), 4096) & VM_W, 0);
	fake_clean("aucune frame touchee");
}

static void	io_map_refus(void)
{
	fake_reset();
	h_true(!vmm_io_map(0xfee00000, 4096, VM_R | VM_X), "X refuse");
	h_true(!vmm_io_map(0xfee00000, 4096, VM_WC | VM_NOCACHE), "UC et WC");
	h_true(!vmm_io_map(0xfee00000, 4096, VM_USER), "USER refuse");
	h_true(!vmm_io_map(0xfee00000, 0, VM_R), "longueur 0");
	h_true(!vmm_io_map(0xfffffffffffff000ull, 0x2000, VM_R), "deborde");
	h_true(!vmm_io_map(PHYS_LIMIT, 4096, VM_R), "hors adresse physique");
	g_vmm.ready = false;
	h_true(!vmm_io_map(0xfee00000, 4096, VM_R), "avant le demarrage");
	g_vmm.ready = true;
	h_true(vmm_io_map(FAKE_BASE + 0x10, 8, VM_R) == phys_to_virt(FAKE_BASE
			+ 0x10), "RAM de la HHDM : alias, un seul type de cache");
	g_vmm.hspan[0].bits = PTE_P;
	h_true(!vmm_io_map(FAKE_BASE, 8, VM_R | VM_W), "ecriture sur HHDM RO");
	h_true(vmm_io_map(FAKE_BASE, 8, VM_R) != NULL, "lecture sur HHDM RO");
	fake_clean("faux pmm propre");
}

static void	pile_garde(void)
{
	uint8_t		*a;
	uint8_t		*b;
	uint64_t	pte;

	fake_reset();
	a = vmm_kstack_alloc(4);
	b = vmm_kstack_alloc(4);
	h_true(a && b && (uintptr_t)a >= KSTACK_BASE + 4096, "deux piles");
	h_true(!fake_pte(&g_vmm.kas, (uintptr_t)a - 4096), "garde sous a");
	h_true(!fake_pte(&g_vmm.kas, (uintptr_t)b - 4096), "garde sous b");
	pte = fake_pte(&g_vmm.kas, (uintptr_t)a + 3 * 4096);
	h_true((pte & PTE_OWNED) && (pte & PTE_W) && (pte & PTE_G), "pile RW");
	h_eq_u64("frames de pile", g_fake.live[PMM_STACK], 8);
	vmm_kstack_free(a, 4);
	vmm_kstack_free(b, 4);
	h_eq_u64("rendues", g_fake.live[PMM_STACK], 0);
	h_eq_u64("tables rendues", g_fake.live[PMM_PAGETABLE], KH_BASELINE);
	h_true(!vmm_kstack_alloc(0) && !vmm_kstack_alloc(257), "tailles");
	vmm_kstack_free((void *)KHEAP_BASE, 4);
	vmm_kstack_free(NULL, 4);
	fake_clean("faux pmm propre");
}

static void	io_unmap_hors_fenetre(void)
{
	uintptr_t	va;

	fake_reset();
	va = vmm_find_free(&g_vmm.kas, 4096, KHEAP_BASE, KHEAP_BASE + WIN_SIZE);
	vmm_alloc(&g_vmm.kas, va, 4096, VM_R | VM_W);
	vmm_io_unmap((void *)va, 4096);
	h_true(fake_pte(&g_vmm.kas, va) != 0, "tas intact");
	vmm_io_unmap((void *)KIO_BASE, 0);
	h_eq_u64("frame de tas", g_fake.live[PMM_HEAP], 1);
	vmm_unmap(&g_vmm.kas, va, 4096);
	h_eq_u64("tas rendu", g_fake.live[PMM_HEAP], 0);
	fake_clean("faux pmm propre");
}

int	main(void)
{
	h_begin("a03/kwin");
	h_run("io_map_decalage", io_map_decalage);
	h_run("io_map_refus", io_map_refus);
	h_run("pile_garde", pile_garde);
	h_run("io_unmap_hors_fenetre", io_unmap_hors_fenetre);
	return (h_end());
}
