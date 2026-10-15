#include <string.h>
#include "velum/err.h"
#include "harness.h"
#include "fake.h"

static void	copie_aller_retour(void)
{
	t_aspace	*as;
	char		in[64];
	char		out[64];

	as = fake_user_env();
	memset(in, 'a', sizeof(in));
	memset(out, 0, sizeof(out));
	h_eq_i64("vers user a cheval", copy_to_user(UVA + 4096 - 20, in, 64), 0);
	h_eq_i64("depuis user", copy_from_user(out, UVA + 4096 - 20, 64), 0);
	h_true(!memcmp(in, out, 64), "contenu identique");
	h_eq_u64("octet vu par la frame", fake_ptr(as, UVA + 4096)[0], 'a');
	h_eq_i64("lecture page RO", copy_from_user(out, UVA + 2 * 4096, 8), 0);
	h_true(user_range_ok(UVA, 3 * 4096), "plage lisible");
	vmm_aspace_destroy(as);
	fake_clean("faux pmm propre");
}

static void	copie_refusee_mcdc(void)
{
	t_aspace	*as;
	char		in[16];

	as = fake_user_env();
	memset(in, 'z', sizeof(in));
	h_eq_i64("U=0", copy_from_user(in, UVA + 4 * 4096, 8), E_FAULT);
	h_eq_i64("U=1 lecture", copy_from_user(in, UVA + 2 * 4096, 8), 0);
	h_eq_i64("U=1 ecriture W=0", copy_to_user(UVA + 2 * 4096, in, 8),
		E_FAULT);
	h_eq_i64("U=1 ecriture W=1", copy_to_user(UVA, in, 8), 0);
	h_eq_u64("RO intact", fake_ptr(as, UVA + 2 * 4096)[0], 0);
	memset(in, 'q', sizeof(in));
	h_eq_i64("a cheval sur non mappee", copy_to_user(UVA + 3 * 4096 - 8, in,
			16), E_FAULT);
	h_eq_i64("a cheval RW puis RO", copy_to_user(UVA + 2 * 4096 - 8, in, 16),
		E_FAULT);
	h_eq_u64("rien ecrit avant", fake_ptr(as, UVA + 2 * 4096 - 8)[0], 0);
	h_true(!user_range_ok(UVA + 3 * 4096 - 8, 16), "plage a cheval");
	vmm_aspace_destroy(as);
	fake_clean("faux pmm propre");
}

static void	copie_limites(void)
{
	t_aspace	*as;
	char		buf[16];

	as = fake_user_env();
	memset(buf, 'k', sizeof(buf));
	h_eq_i64("taille 0", copy_from_user(buf, 0, 0), 0);
	h_eq_u64("intact", (uint64_t)buf[0], 'k');
	h_eq_i64("adresse 0", copy_from_user(buf, 0, 1), E_FAULT);
	h_eq_i64("deborde 64 bits", copy_from_user(buf, 0xfffffffffffffff0ull,
			0x20), E_FAULT);
	h_eq_i64("depasse USER_TOP", copy_from_user(buf, USER_TOP - 4, 8),
		E_FAULT);
	h_eq_i64("noyau", copy_from_user(buf, KERNEL_BASE, 8), E_FAULT);
	h_eq_i64("non canonique", copy_from_user(buf, 0x0000800000000000ull, 8),
		E_FAULT);
	h_eq_i64("tampon nul", copy_from_user(NULL, UVA, 8), E_FAULT);
	h_eq_i64("source nulle", copy_to_user(UVA, NULL, 8), E_FAULT);
	h_true(user_range_ok(0, 0), "plage vide");
	vmm_switch(NULL);
	h_eq_i64("espace noyau courant", copy_from_user(buf, UVA, 8), E_FAULT);
	vmm_aspace_destroy(as);
	fake_clean("faux pmm propre");
}

static void	copie_hors_hhdm(void)
{
	t_aspace	*as;
	t_vmreq		rq;
	char		buf[8];

	as = fake_user_env();
	rq.va = 0x500000;
	rq.pa = 0xfd000000;
	rq.len = 4096;
	rq.flags = VM_R | VM_W | VM_WC | VM_USER;
	h_eq_i64("map mmio", vmm_map(as, &rq), 0);
	h_eq_i64("lecture refusee", copy_from_user(buf, 0x500000, 8), E_FAULT);
	h_eq_i64("ecriture refusee", copy_to_user(0x500000, buf, 8), E_FAULT);
	vmm_aspace_destroy(as);
	fake_clean("aucun acces hors arene");
}

int	main(void)
{
	h_begin("a03/user");
	h_run("copie_aller_retour", copie_aller_retour);
	h_run("copie_refusee_mcdc", copie_refusee_mcdc);
	h_run("copie_limites", copie_limites);
	h_run("copie_hors_hhdm", copie_hors_hhdm);
	return (h_end());
}
