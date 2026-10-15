#include "harness.h"
#include "fake.h"

static void	premier_trou(void)
{
	t_aspace	*as;

	fake_reset();
	as = fake_user();
	h_eq_u64("vide", vmm_find_free(as, 4096, UVA, USER_TOP), UVA);
	vmm_alloc(as, UVA, 2 * 4096, VM_R | VM_USER);
	vmm_alloc(as, UVA + 3 * 4096, 4096, VM_R | VM_USER);
	h_eq_u64("une page", vmm_find_free(as, 4096, UVA, USER_TOP),
		UVA + 2 * 4096);
	h_eq_u64("deux pages", vmm_find_free(as, 2 * 4096, UVA, USER_TOP),
		UVA + 4 * 4096);
	h_eq_u64("ajuste exact", vmm_find_free(as, 4096, UVA, UVA + 3 * 4096),
		UVA + 2 * 4096);
	h_eq_u64("depart dans region", vmm_find_free(as, 4096, UVA + 4096,
			USER_TOP), UVA + 2 * 4096);
	vmm_aspace_destroy(as);
	fake_clean("faux pmm propre");
}

static void	bornes(void)
{
	t_aspace	*as;

	fake_reset();
	as = fake_user();
	h_eq_u64("longueur 0", vmm_find_free(as, 0, UVA, USER_TOP), 0);
	h_eq_u64("non alignee", vmm_find_free(as, 100, UVA, USER_TOP), 0);
	h_eq_u64("lo > hi", vmm_find_free(as, 4096, UVA, UVA - 4096), 0);
	h_eq_u64("trop petit", vmm_find_free(as, 2 * 4096, UVA, UVA + 4096), 0);
	h_eq_u64("lo arrondi", vmm_find_free(as, 4096, UVA + 1, USER_TOP),
		UVA + 4096);
	h_eq_u64("plancher USER_MIN", vmm_find_free(as, 4096, 0, USER_TOP),
		USER_MIN);
	h_eq_u64("plafond", vmm_find_free(as, 4096, USER_TOP - 4096,
			0xffffffffffffffffull), USER_TOP - 4096);
	h_eq_u64("nul", vmm_find_free(NULL, 4096, UVA, USER_TOP), 0);
	vmm_aspace_destroy(as);
	fake_clean("faux pmm propre");
}

int	main(void)
{
	h_begin("a03/find");
	h_run("premier_trou", premier_trou);
	h_run("bornes", bornes);
	return (h_end());
}
