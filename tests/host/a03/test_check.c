#include "velum/err.h"
#include "harness.h"
#include "fake.h"

static void	limites_adresses(void)
{
	t_aspace	*as;

	fake_reset();
	as = fake_user();
	h_eq_i64("va 0", vmm_check_range(as, 0, 4096), E_INVAL);
	h_eq_i64("va 4095", vmm_check_range(as, 4095, 4096), E_INVAL);
	h_eq_i64("sous USER_MIN", vmm_check_range(as, USER_MIN - 4096, 4096),
		E_INVAL);
	h_eq_i64("USER_MIN", vmm_check_range(as, USER_MIN, 4096), 0);
	h_eq_i64("derniere page", vmm_check_range(as, USER_TOP - 4096, 4096), 0);
	h_eq_i64("deborde USER_TOP", vmm_check_range(as, USER_TOP - 4096, 8192),
		E_INVAL);
	h_eq_i64("USER_TOP", vmm_check_range(as, USER_TOP, 4096), E_INVAL);
	h_eq_i64("non canonique", vmm_check_range(as, 0x0000800000000000ull,
			4096), E_INVAL);
	h_eq_i64("moitie haute", vmm_check_range(as, KHEAP_BASE, 4096), E_INVAL);
	h_eq_i64("espace nul", vmm_check_range(NULL, UVA, 4096), E_INVAL);
	vmm_aspace_destroy(as);
	fake_clean("faux pmm propre");
}

static void	limites_longueurs(void)
{
	t_aspace	*as;

	fake_reset();
	as = fake_user();
	h_eq_i64("longueur 0", vmm_check_range(as, UVA, 0), E_INVAL);
	h_eq_i64("longueur 4095", vmm_check_range(as, UVA, 4095), E_INVAL);
	h_eq_i64("longueur 4096", vmm_check_range(as, UVA, 4096), 0);
	h_eq_i64("longueur 4097", vmm_check_range(as, UVA, 4097), E_INVAL);
	h_eq_i64("debordement 64 bits", vmm_check_range(as,
			0xfffffffffffff000ull, 0x2000), E_INVAL);
	h_eq_i64("alloc longueur 0", vmm_alloc(as, UVA, 0, VM_R), E_INVAL);
	h_eq_i64("alloc non alignee", vmm_alloc(as, UVA, 100, VM_R), E_INVAL);
	h_eq_u64("rien alloue", g_fake.live[PMM_USER], 0);
	vmm_aspace_destroy(as);
	fake_clean("faux pmm propre");
}

static void	table_drapeaux(void)
{
	t_aspace	*k;
	t_aspace	u;

	fake_reset();
	k = &g_vmm.kas;
	u.kernel = false;
	h_eq_i64("bit inconnu", vmm_check_flags(k, VM_R | 0x100, false), E_INVAL);
	h_eq_i64("R absent", vmm_check_flags(k, VM_W, false), E_INVAL);
	h_eq_i64("W et X", vmm_check_flags(k, VM_R | VM_W | VM_X, false), E_INVAL);
	h_eq_i64("UC et WC", vmm_check_flags(k, VM_R | VM_NOCACHE | VM_WC,
			false), E_INVAL);
	h_eq_i64("anonyme WC", vmm_check_flags(k, VM_R | VM_WC, true), E_INVAL);
	h_eq_i64("anonyme UC", vmm_check_flags(k, VM_R | VM_NOCACHE, true),
		E_INVAL);
	h_eq_i64("anonyme partage", vmm_check_flags(&u, VM_R | VM_SHARED, true),
		E_INVAL);
	h_eq_i64("noyau USER", vmm_check_flags(k, VM_R | VM_USER, false), E_INVAL);
	h_eq_i64("user GLOBAL", vmm_check_flags(&u, VM_R | VM_GLOBAL, false),
		E_INVAL);
	h_eq_i64("noyau RW G", vmm_check_flags(k, VM_R | VM_W | VM_GLOBAL, true),
		0);
	h_eq_i64("user RX", vmm_check_flags(&u, VM_R | VM_X | VM_USER, true), 0);
	h_eq_i64("map WC partage", vmm_check_flags(&u, VM_R | VM_W | VM_WC
			| VM_SHARED | VM_USER, false), 0);
}

static void	fenetres_noyau(void)
{
	t_aspace	*k;

	fake_reset();
	k = &g_vmm.kas;
	h_eq_i64("debut tas", vmm_check_range(k, KHEAP_BASE, 4096), 0);
	h_eq_i64("fin tas", vmm_check_range(k, KHEAP_BASE + WIN_SIZE - 4096,
			4096), 0);
	h_eq_i64("deborde tas", vmm_check_range(k, KHEAP_BASE + WIN_SIZE - 4096,
			8192), E_INVAL);
	h_eq_i64("trou entre fenetres", vmm_check_range(k, KHEAP_BASE + WIN_SIZE,
			4096), E_INVAL);
	h_eq_i64("fenetre io", vmm_check_range(k, KIO_BASE, 4096), 0);
	h_eq_i64("fenetre piles", vmm_check_range(k, KSTACK_BASE, 4096), 0);
	h_eq_i64("hhdm", vmm_check_range(k, HHDM_DEFAULT, 4096), E_INVAL);
	h_eq_i64("image", vmm_check_range(k, KERNEL_BASE, 4096), E_INVAL);
	h_eq_i64("user dans noyau", vmm_check_range(k, UVA, 4096), E_INVAL);
	h_eq_i64("alloc hhdm refusee", vmm_alloc(k, HHDM_DEFAULT, 4096, VM_R),
		E_INVAL);
	fake_clean("faux pmm propre");
}

int	main(void)
{
	h_begin("a03/check");
	h_run("limites_adresses", limites_adresses);
	h_run("limites_longueurs", limites_longueurs);
	h_run("table_drapeaux", table_drapeaux);
	h_run("fenetres_noyau", fenetres_noyau);
	return (h_end());
}
