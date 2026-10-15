#include <stdint.h>
#include "harness.h"
#include "velum/libk.h"
#include "a20_test.h"
#include "accounts.h"
#include "a20_acc.h"

static void	verify_a_vide_et_b_motdepasse(void)
{
	t_account	a;
	t_account	b;
	const char	*la;
	const char	*lb;

	la = "Utilisateur:10000:" V_SALT_A ":" V_HA1 V_HA2 ":0";
	lb = "Alice:12000:" V_SALT_B ":" V_HB1 V_HB2 ":0";
	h_eq_i64("A ligne", acc_load(&a, la), 0);
	h_eq_i64("B ligne", acc_load(&b, lb), 0);
	h_eq_i64("A vide", acc_verify(&a, "", 0), 1);
	h_eq_i64("A pointeur nul vide", acc_verify(&a, NULL, 0), 1);
	h_eq_i64("A espace", acc_verify(&a, " ", 1), 0);
	h_eq_i64("A sans mot de passe requis", acc_needs_password(&a), 0);
	h_eq_i64("B bon", acc_verify(&b, "motdepasse", 10), 1);
	h_eq_i64("B casse", acc_verify(&b, "Motdepasse", 10), 0);
	h_eq_i64("B espace final", acc_verify(&b, "motdepasse ", 11), 0);
	h_eq_i64("B prefixe", acc_verify(&b, "motdepass", 9), 0);
	h_eq_i64("B vide", acc_verify(&b, "", 0), 0);
	h_eq_i64("B mot de passe requis", acc_needs_password(&b), 1);
}

static void	verify_c_d_e_longueurs_et_utf8(void)
{
	static char	pw[ACC_PW_MAX + 8];
	t_account	c;
	t_account	d;
	t_account	e;

	h_eq_i64("C", acc_load(&c, "U:10000:" V_SALT_C1 V_SALT_C2 ":" V_HC1
			V_HC2 ":0"), 0);
	memset(pw, 'x', sizeof(pw));
	h_eq_i64("C 200", acc_verify(&c, pw, 200), 1);
	h_eq_i64("C 199", acc_verify(&c, pw, 199), 0);
	h_eq_i64("C 201", acc_verify(&c, pw, 201), 0);
	h_eq_i64("D", acc_load(&d, "U:10000:" V_SALT_D ":" V_HD1 V_HD2 ":0"), 0);
	h_eq_i64("D utf8", acc_verify(&d, "\303\251t\303\251-\342\202\254", 9), 1);
	h_eq_i64("D sans accent", acc_verify(&d, "ete-EUR", 7), 0);
	h_eq_i64("E", acc_load(&e, "U:10000:" V_SALT_E ":" V_HE1 V_HE2 ":0"), 0);
	memset(pw, 'a', sizeof(pw));
	h_eq_i64("E 1024 pile", acc_verify(&e, pw, ACC_PW_MAX), 1);
	h_eq_i64("E 1025 refuse", acc_verify(&e, pw, ACC_PW_MAX + 1), 0);
	h_eq_i64("E 1032 refuse", acc_verify(&e, pw, ACC_PW_MAX + 8), 0);
}

static void	verify_nul_integre_et_mutations(void)
{
	t_account	n;
	t_account	s;
	t_account	a;
	size_t		i;

	h_eq_i64("N", acc_load(&n, "U:10000:" V_SALT_N ":" V_HN1 V_HN2 ":0"), 0);
	h_eq_i64("S", acc_load(&s, "U:10000:" V_SALT_N ":" V_HS1 V_HS2 ":0"), 0);
	h_eq_i64("N avec NUL integre", acc_verify(&n, "ab\0c", 4), 1);
	h_eq_i64("N coupe au NUL", acc_verify(&n, "ab", 2), 0);
	h_eq_i64("S ab", acc_verify(&s, "ab", 2), 1);
	h_eq_i64("S ab NUL c", acc_verify(&s, "ab\0c", 4), 0);
	h_eq_i64("A", acc_load(&a, "U:10000:" V_SALT_A ":" V_HA1 V_HA2 ":0"), 0);
	i = 0;
	while (i < SHA256_LEN)
	{
		a.hash[i] ^= 1;
		h_eq_i64("un bit du hash change", acc_verify(&a, "", 0), 0);
		a.hash[i] ^= 1;
		i++;
	}
}

static void	verify_entrees_invalides(void)
{
	t_account	a;

	memset(&a, 0, sizeof(a));
	h_eq_i64("compte nul", acc_verify(NULL, "", 0), 0);
	h_eq_i64("mot de passe nul avec taille", acc_verify(&a, NULL, 3), 0);
	a.iterations = 0;
	a.salt_len = 8;
	h_eq_i64("iterations nulles", acc_verify(&a, "", 0), -22);
	h_eq_i64("echec de derivation exige un mot de passe",
		acc_needs_password(&a), 1);
	a.iterations = ACC_ITER_MIN;
	h_eq_i64("hash nul n'accepte pas le vide", acc_verify(&a, "", 0), 0);
}

int	main(void)
{
	h_begin("a20/acc-verify");
	h_run("verify: A vide, B mot de passe", verify_a_vide_et_b_motdepasse);
	h_run("verify: C, D, E longueurs et UTF-8", verify_c_d_e_longueurs_et_utf8);
	h_run("verify: NUL integre et mutations", verify_nul_integre_et_mutations);
	h_run("verify: entrees invalides", verify_entrees_invalides);
	return (h_end());
}
