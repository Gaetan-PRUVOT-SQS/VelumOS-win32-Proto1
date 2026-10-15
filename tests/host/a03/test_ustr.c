#include <string.h>
#include "velum/err.h"
#include "harness.h"
#include "fake.h"

static t_aspace	*str_env(const char *s, uintptr_t va)
{
	t_aspace	*as;

	fake_reset();
	as = fake_user();
	vmm_alloc(as, UVA, 2 * 4096, VM_R | VM_W | VM_USER);
	copy_to_user(va, s, strlen(s) + 1);
	return (as);
}

static void	chaine_normale(void)
{
	t_aspace	*as;
	char		dst[32];

	as = str_env("bonjour", UVA + 4096 - 3);
	h_eq_i64("longueur", strncpy_from_user(dst, UVA + 4096 - 3, 32), 7);
	h_eq_str("contenu a cheval", dst, "bonjour");
	h_eq_i64("max juste", strncpy_from_user(dst, UVA + 4096 - 3, 8), 7);
	h_eq_i64("max trop court", strncpy_from_user(dst, UVA + 4096 - 3, 7),
		E_RANGE);
	h_eq_str("vide apres echec", dst, "");
	h_eq_i64("max 0", strncpy_from_user(dst, UVA, 0), E_RANGE);
	h_eq_i64("dst nul", strncpy_from_user(NULL, UVA, 4), E_FAULT);
	vmm_aspace_destroy(as);
	fake_clean("faux pmm propre");
}

static void	chaine_bord_de_page(void)
{
	t_aspace	*as;
	char		dst[32];
	uint8_t		*last;

	as = str_env("abc", UVA + 2 * 4096 - 4);
	h_eq_i64("NUL au dernier octet", strncpy_from_user(dst,
			UVA + 2 * 4096 - 4, 32), 3);
	last = fake_ptr(as, UVA + 2 * 4096 - 1);
	*last = 'd';
	h_eq_i64("deborde sur non mappee", strncpy_from_user(dst,
			UVA + 2 * 4096 - 4, 32), E_FAULT);
	h_eq_str("vide apres faute", dst, "");
	h_eq_i64("noyau", strncpy_from_user(dst, KERNEL_BASE, 32), E_FAULT);
	h_eq_i64("deborde 64 bits", strncpy_from_user(dst,
			0xfffffffffffffffeull, 32), E_FAULT);
	vmm_switch(NULL);
	h_eq_i64("espace noyau", strncpy_from_user(dst, UVA, 32), E_FAULT);
	vmm_aspace_destroy(as);
	fake_clean("faux pmm propre");
}

static void	plage_utilisateur(void)
{
	t_aspace	*as;

	as = str_env("", UVA);
	h_true(user_range_ok(UVA, 2 * 4096), "deux pages");
	h_true(!user_range_ok(UVA, 2 * 4096 + 1), "un octet de trop");
	h_true(!user_range_ok(0xfffffffffffff000ull, 0x2000), "deborde");
	h_true(!user_range_ok(UVA - 1, 2), "avant la region");
	vmm_switch(NULL);
	h_true(!user_range_ok(UVA, 1), "espace noyau");
	vmm_aspace_destroy(as);
	fake_clean("faux pmm propre");
}

int	main(void)
{
	h_begin("a03/ustr");
	h_run("chaine_normale", chaine_normale);
	h_run("chaine_bord_de_page", chaine_bord_de_page);
	h_run("plage_utilisateur", plage_utilisateur);
	return (h_end());
}
