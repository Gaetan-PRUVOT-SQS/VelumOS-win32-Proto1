#include "velum/err.h"
#include "harness.h"
#include "fake.h"

#define RW (VM_R | VM_W | VM_USER)

static t_aspace	*espace_sur_deux_coeurs(uint32_t ncpu)
{
	t_aspace	*as;

	fake_reset();
	g_fake.ncpu = ncpu;
	as = fake_user();
	fake_cpu_set(1);
	vmm_switch(as);
	fake_cpu_set(0);
	return (as);
}

static void	un_coeur_sans_ipi(void)
{
	t_aspace	*as;

	fake_reset();
	as = fake_user();
	vmm_alloc(as, UVA, 4 * 4096, RW);
	vmm_unmap(as, UVA, 2 * 4096);
	vmm_protect(as, UVA + 2 * 4096, 4096, VM_R);
	h_eq_u64("aucun ipi", g_fake.ipis, 0);
	h_eq_u64("aucun tir", g_vmm.tlb.shots, 0);
	h_eq_u64("invlpg locaux", g_fake.invlpg_cpu[0], 3);
	vmm_aspace_destroy(as);
	fake_clean("faux pmm propre");
}

static void	unmap_invalide_le_coeur_distant(void)
{
	t_aspace	*as;

	as = espace_sur_deux_coeurs(3);
	vmm_alloc(as, UVA, 4 * 4096, RW);
	fake_touch(1, as, UVA);
	fake_touch(1, as, UVA + 4096);
	fake_touch(1, as, UVA + 3 * 4096);
	h_eq_u64("masque", as->cpumask, 0x3);
	h_eq_i64("unmap", vmm_unmap(as, UVA, 2 * 4096), 0);
	h_true(!fake_stale(1, UVA) && !fake_stale(1, UVA + 4096), "invalide");
	h_true(fake_stale(1, UVA + 3 * 4096), "page voisine gardee");
	h_eq_u64("invlpg cpu1", g_fake.invlpg_cpu[1], 2);
	h_eq_u64("cpu2 non sollicite", g_fake.invlpg_cpu[2], 0);
	h_eq_u64("un seul ipi", g_fake.ipis, 1);
	h_eq_u64("un seul tir", g_vmm.tlb.shots, 1);
	h_eq_u64("rien en attente", g_vmm.tlb.pending, 0);
	fake_cpu_set(1);
	vmm_switch(NULL);
	fake_cpu_set(0);
	vmm_aspace_destroy(as);
	fake_clean("faux pmm propre");
}

static void	frame_liberee_apres_invalidation(void)
{
	t_aspace	*as;
	uint64_t	pa;

	as = espace_sur_deux_coeurs(2);
	vmm_alloc(as, UVA, 3 * 4096, RW);
	fake_touch(1, as, UVA);
	fake_touch(1, as, UVA + 4096);
	fake_touch(1, as, UVA + 2 * 4096);
	vmm_unmap(as, UVA, 3 * 4096);
	h_eq_u64("aucun frame libere en cache distant", g_fake.stale_free, 0);
	fake_cpu_set(1);
	vmm_switch(NULL);
	fake_cpu_set(0);
	vmm_alloc(as, UVA, 4096, RW);
	pa = fake_pte(as, UVA) & PTE_ADDR;
	fake_touch(1, as, UVA);
	fake_check_stale(pa);
	h_eq_u64("temoin: le detecteur voit un cache perime", g_fake.stale_free, 1);
	g_fake.stale_free = 0;
	g_fake.tlb_pa[1][0] = 0;
	vmm_aspace_destroy(as);
	fake_clean("faux pmm propre");
}

static void	protect_invalide_le_coeur_distant(void)
{
	t_aspace	*as;

	as = espace_sur_deux_coeurs(2);
	vmm_alloc(as, UVA, 2 * 4096, RW);
	fake_touch(1, as, UVA);
	fake_touch(1, as, UVA + 4096);
	h_eq_i64("protect", vmm_protect(as, UVA, 4096, VM_R), 0);
	h_true(!fake_stale(1, UVA), "droits perimes retires");
	h_true(fake_stale(1, UVA + 4096), "page voisine gardee");
	h_eq_u64("invlpg cpu1", g_fake.invlpg_cpu[1], 1);
	fake_cpu_set(1);
	vmm_switch(NULL);
	fake_cpu_set(0);
	vmm_aspace_destroy(as);
	fake_clean("faux pmm propre");
}

static void	lots_de_32_pages(void)
{
	t_aspace	*as;

	as = espace_sur_deux_coeurs(2);
	vmm_alloc(as, UVA, 70 * 4096, RW);
	h_eq_i64("unmap 70 pages", vmm_unmap(as, UVA, 70 * 4096), 0);
	h_eq_u64("trois lots (32, 32, 6) et trois tables elaguees",
		g_vmm.tlb.shots, 3 + 3);
	h_eq_u64("70 pages et 3 tables", g_fake.invlpg_cpu[1], 70 + 3);
	h_eq_u64("pas de vidage complet", g_fake.flush_cpu[1], 0);
	h_eq_u64("frames rendues", g_fake.live[PMM_USER], 0);
	fake_cpu_set(1);
	vmm_switch(NULL);
	fake_cpu_set(0);
	vmm_aspace_destroy(as);
	fake_clean("faux pmm propre");
}

static void	intervalle_large_vidage_complet(void)
{
	t_aspace	*as;

	as = espace_sur_deux_coeurs(2);
	vmm_alloc(as, UVA, 4096, RW);
	vmm_alloc(as, UVA + 100 * 4096, 4096, RW);
	h_eq_i64("unmap", vmm_unmap(as, UVA, 101 * 4096), 0);
	h_eq_u64("un vidage complet", g_fake.flush_cpu[1], 1);
	h_eq_u64("seules les 3 tables elaguees en invlpg", g_fake.invlpg_cpu[1], 3);
	fake_cpu_set(1);
	vmm_switch(NULL);
	fake_cpu_set(0);
	vmm_aspace_destroy(as);
	fake_clean("faux pmm propre");
}

static void	masque_suit_les_changements_d_espace(void)
{
	t_aspace	*as;
	t_aspace	*other;

	as = espace_sur_deux_coeurs(2);
	h_eq_u64("cpu0 et cpu1", as->cpumask, 0x3);
	other = vmm_aspace_create();
	fake_cpu_set(1);
	vmm_switch(other);
	h_eq_u64("cpu1 quitte", as->cpumask, 0x1);
	h_eq_u64("cpu1 arrive", other->cpumask, 0x2);
	vmm_switch(NULL);
	h_eq_u64("retour au noyau", other->cpumask, 0);
	vmm_switch(as);
	fake_cpu_set(0);
	vmm_aspace_destroy(as);
	h_true(g_fake.errors == 1, "destruction d'un espace charge ailleurs signalee");
	g_fake.errors = 0;
	g_vmm.tlb.cur[1] = NULL;
	vmm_aspace_destroy(other);
	fake_clean("faux pmm propre");
}

static void	espace_noyau_tous_les_coeurs(void)
{
	void	*stack;

	fake_reset();
	g_fake.ncpu = 3;
	stack = vmm_kstack_alloc(2);
	h_true(stack != NULL, "pile noyau");
	fake_touch(1, vmm_kernel_aspace(), (uintptr_t)stack);
	fake_touch(2, vmm_kernel_aspace(), (uintptr_t)stack + 4096);
	vmm_kstack_free(stack, 2);
	h_eq_u64("trois tirs: pages, table de pages, repertoire", g_vmm.tlb.shots, 3);
	h_eq_u64("deux ipi par tir", g_fake.ipis, 3 * 2);
	h_true(!fake_stale(1, (uintptr_t)stack), "cpu1 invalide");
	h_true(!fake_stale(2, (uintptr_t)stack + 4096), "cpu2 invalide");
	h_eq_u64("aucun frame libere en cache", g_fake.stale_free, 0);
	fake_clean("faux pmm propre");
}

static void	elagage_attend_les_coeurs_distants(void)
{
	t_aspace	*as;
	uintptr_t	va;

	as = espace_sur_deux_coeurs(2);
	va = 0x7f0000000000ull;
	vmm_alloc(as, va, 4096, RW);
	h_eq_u64("tables creees", as->tables, 4);
	vmm_unmap(as, va, 4096);
	h_eq_u64("tables elaguees", as->tables, 1);
	h_eq_u64("un tir pour la page et un par table", g_vmm.tlb.shots, 4);
	h_eq_u64("tables pmm", g_fake.live[PMM_PAGETABLE], KH_BASELINE + 1);
	fake_cpu_set(1);
	vmm_switch(NULL);
	fake_cpu_set(0);
	vmm_aspace_destroy(as);
	fake_clean("faux pmm propre");
}

static void	ipi_perdu_ne_bloque_pas(void)
{
	t_aspace	*as;

	as = espace_sur_deux_coeurs(2);
	vmm_alloc(as, UVA, 4096, RW);
	g_fake.drop_ipi = true;
	g_vmm.tlb.spin_max = 1000;
	h_eq_i64("unmap rend la main", vmm_unmap(as, UVA, 4096), 0);
	h_eq_u64("un delai par tir: page et trois tables", g_vmm.tlb.timeouts, 1 + 3);
	h_eq_u64("plus rien en attente", g_vmm.tlb.pending, 0);
	g_fake.drop_ipi = false;
	fake_cpu_set(1);
	vmm_switch(NULL);
	fake_cpu_set(0);
	vmm_aspace_destroy(as);
	fake_clean("faux pmm propre");
}

static t_aspace	*g_attendu;
static uint64_t	g_pending_vu = 99;

static void	liberer_le_verrou(void)
{
	g_pending_vu = g_vmm.tlb.pending;
	__atomic_store_n(&g_attendu->serving, g_attendu->serving + 1,
		__ATOMIC_RELEASE);
	g_fake.relax_hook = NULL;
}

static void	verrou_traite_les_demandes_en_attente(void)
{
	t_aspace	*as;
	uint64_t	irq;

	as = espace_sur_deux_coeurs(2);
	fake_touch(0, as, UVA);
	g_vmm.tlb.req.lo = UVA;
	g_vmm.tlb.req.hi = UVA + 4096;
	g_vmm.tlb.pending = 1;
	as->ticket = 1;
	as->serving = 0;
	g_attendu = as;
	g_fake.relax_hook = liberer_le_verrou;
	irq = vmm_lock(as);
	h_eq_u64("demande traitee pendant l'attente du verrou", g_pending_vu, 0);
	h_eq_u64("invlpg du coeur courant", g_fake.invlpg_cpu[0], 1);
	h_true(!fake_stale(0, UVA), "entree retiree");
	vmm_unlock(as, irq);
	fake_cpu_set(1);
	vmm_switch(NULL);
	fake_cpu_set(0);
	vmm_aspace_destroy(as);
}

int	main(void)
{
	h_begin("a03/smp");
	h_run("un_coeur_sans_ipi", un_coeur_sans_ipi);
	h_run("unmap_invalide_le_coeur_distant", unmap_invalide_le_coeur_distant);
	h_run("frame_liberee_apres_invalidation",
		frame_liberee_apres_invalidation);
	h_run("protect_invalide_le_coeur_distant",
		protect_invalide_le_coeur_distant);
	h_run("lots_de_32_pages", lots_de_32_pages);
	h_run("intervalle_large_vidage_complet", intervalle_large_vidage_complet);
	h_run("masque_suit_les_changements_d_espace",
		masque_suit_les_changements_d_espace);
	h_run("espace_noyau_tous_les_coeurs", espace_noyau_tous_les_coeurs);
	h_run("elagage_attend_les_coeurs_distants",
		elagage_attend_les_coeurs_distants);
	h_run("ipi_perdu_ne_bloque_pas", ipi_perdu_ne_bloque_pas);
	h_run("verrou_traite_les_demandes_en_attente",
		verrou_traite_les_demandes_en_attente);
	return (h_end());
}
