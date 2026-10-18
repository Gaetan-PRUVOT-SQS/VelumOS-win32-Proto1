#include <string.h>
#include "harness.h"
#include "cpu_int.h"
#include "spec_apply.h"

#define B(n) (1ull << (n))

static uint64_t	g_ecrit[8];
static uint32_t	g_ecrit_msr[8];
static int		g_ecritures;
static uint64_t	g_relu;
static int		g_lectures;

static void	faux_ecrit(uint32_t msr, uint64_t valeur)
{
	if (g_ecritures < 8)
	{
		g_ecrit_msr[g_ecritures] = msr;
		g_ecrit[g_ecritures] = valeur;
	}
	g_ecritures++;
	g_relu = valeur;
}

static void	faux_ecrit_masque(uint32_t msr, uint64_t valeur)
{
	faux_ecrit(msr, valeur);
	g_relu = valeur & ~SPEC_CTRL_SSBD;
}

static uint64_t	faux_lit(uint32_t msr)
{
	g_lectures++;
	h_eq_u64("relecture sur le msr 0x48", msr, 0x48);
	return (g_relu);
}

static void	remet_a_zero(void)
{
	memset(g_ecrit, 0, sizeof(g_ecrit));
	memset(g_ecrit_msr, 0, sizeof(g_ecrit_msr));
	g_ecritures = 0;
	g_lectures = 0;
	g_relu = 0;
}

static void	constantes_msr(void)
{
	h_eq_u64("IA32_SPEC_CTRL numero 0x48", MSR_SPEC_CTRL, 0x48);
	h_eq_u64("IBRS bit 0", SPEC_CTRL_IBRS, B(0));
	h_eq_u64("STIBP bit 1", SPEC_CTRL_STIBP, B(1));
	h_eq_u64("SSBD bit 2", SPEC_CTRL_SSBD, B(2));
}

typedef struct s_masquecas
{
	const char	*nom;
	bool		spec_ctrl;
	bool		stibp;
	bool		ssbd;
	bool		amd_ibrs;
	bool		amd_stibp;
	bool		amd_ssbd;
	uint64_t	attendu;
}	t_masquecas;

static const t_masquecas	g_masques[] = {
{"M1 aucun controle", false, false, false, false, false, false, 0},
{"M2 intel spec_ctrl", true, false, false, false, false, false, B(0)},
{"M3 intel stibp seul", false, true, false, false, false, false, B(1)},
{"M4 intel ssbd seul", false, false, true, false, false, false, B(2)},
{"M5 intel tout", true, true, true, false, false, false, B(0) | B(1) | B(2)},
{"M6 amd ibrs", false, false, false, true, false, false, B(0)},
{"M7 amd stibp et ssbd", false, false, false, false, true, true,
	B(1) | B(2)},
{"M8 amd tout", false, false, false, true, true, true, B(0) | B(1) | B(2)},
{NULL, false, false, false, false, false, false, 0}
};

static void	masque_par_capacite(void)
{
	t_cpuvuln	v;
	int			i;

	i = 0;
	while (g_masques[i].nom)
	{
		memset(&v, 0, sizeof(v));
		v.spec_ctrl = g_masques[i].spec_ctrl;
		v.stibp = g_masques[i].stibp;
		v.ssbd = g_masques[i].ssbd;
		v.amd_ibrs = g_masques[i].amd_ibrs;
		v.amd_stibp = g_masques[i].amd_stibp;
		v.amd_ssbd = g_masques[i].amd_ssbd;
		h_eq_u64(g_masques[i].nom, spec_msr_mask(&v), g_masques[i].attendu);
		i++;
	}
}

static void	cible_intel_ssbd(void)
{
	t_cpuvuln	v;
	t_specid	id;
	t_specplan	plan;

	memset(&v, 0, sizeof(v));
	memset(&id, 0, sizeof(id));
	id.vendor = SPEC_VENDOR_INTEL;
	id.htt = true;
	v.spec_ctrl = true;
	v.stibp = true;
	v.ssbd = true;
	spec_plan_decide(&v, &id, &plan);
	h_eq_u64("stibp et ssbd demandes", spec_msr_target(&v, &plan, true),
		B(1) | B(2));
	h_eq_u64("ssbd refuse par l'option", spec_msr_target(&v, &plan, false),
		B(1));
}

static void	cible_eibrs_et_ibrs_entree(void)
{
	t_cpuvuln	v;
	t_specid	id;
	t_specplan	plan;

	memset(&v, 0, sizeof(v));
	memset(&id, 0, sizeof(id));
	id.vendor = SPEC_VENDOR_INTEL;
	v.spec_ctrl = true;
	v.arch_cap = true;
	v.arch_cap_read = true;
	v.arch_cap_value = B(1) | B(4);
	spec_plan_decide(&v, &id, &plan);
	h_eq_u64("eibrs, ssb_no, ibrs permanent", spec_msr_target(&v, &plan, true),
		B(0));
	v.arch_cap_value = B(2);
	spec_plan_decide(&v, &id, &plan);
	h_eq_u64("ibrs a l'entree appliqué en permanence",
		spec_msr_target(&v, &plan, true), B(0));
}

static void	cible_amd(void)
{
	t_cpuvuln	v;
	t_specid	id;
	t_specplan	plan;

	memset(&v, 0, sizeof(v));
	memset(&id, 0, sizeof(id));
	id.vendor = SPEC_VENDOR_AMD;
	id.htt = true;
	v.amd_stibp = true;
	v.amd_ssbd = true;
	spec_plan_decide(&v, &id, &plan);
	h_eq_u64("amd stibp et ssbd", spec_msr_target(&v, &plan, true),
		B(1) | B(2));
	id.amd_ssb_no = true;
	spec_plan_decide(&v, &id, &plan);
	h_eq_u64("amd ssb_no, pas de ssbd", spec_msr_target(&v, &plan, true),
		B(1));
	memset(&v, 0, sizeof(v));
	spec_plan_decide(&v, &id, &plan);
	h_eq_u64("amd sans controle", spec_msr_target(&v, &plan, true), 0);
}

static void	cible_ne_depasse_jamais_les_capacites(void)
{
	t_cpuvuln	v;
	t_specplan	plan;

	memset(&v, 0, sizeof(v));
	memset(&plan, 0, sizeof(plan));
	plan.spec_ctrl_kernel = 7;
	plan.ssb = SPEC_CTRL_AVAIL;
	h_eq_u64("plan trop ambitieux, cpu sans bits", spec_msr_target(&v, &plan,
			true), 0);
	v.stibp = true;
	h_eq_u64("seul stibp survit", spec_msr_target(&v, &plan, true), B(1));
}

static void	application_ecriture_et_relecture(void)
{
	remet_a_zero();
	h_eq_i64("cible nulle, rien", spec_msr_apply(faux_ecrit, faux_lit, 0),
		SPEC_APPLY_NONE);
	h_eq_i64("cible nulle, aucune ecriture", g_ecritures, 0);
	h_eq_i64("ecriture acceptee", spec_msr_apply(faux_ecrit, faux_lit, 6),
		SPEC_APPLY_OK);
	h_eq_i64("une seule ecriture", g_ecritures, 1);
	h_eq_u64("ecrit dans le msr 0x48", g_ecrit_msr[0], 0x48);
	h_eq_u64("valeur ecrite", g_ecrit[0], 6);
	h_eq_i64("une relecture", g_lectures, 1);
}

static void	application_refusee_par_l_hyperviseur(void)
{
	remet_a_zero();
	h_eq_i64("bit ssbd ignore par l'hyperviseur", spec_msr_apply(
			faux_ecrit_masque, faux_lit, 6), SPEC_APPLY_REFUSED);
	h_eq_i64("l'ecriture a bien eu lieu", g_ecritures, 1);
	remet_a_zero();
	h_eq_i64("bits acceptes", spec_msr_apply(faux_ecrit_masque, faux_lit, 3),
		SPEC_APPLY_OK);
}

int	main(void)
{
	h_begin("a01/spec_apply");
	h_run("constantes msr", constantes_msr);
	h_run("masque par capacite", masque_par_capacite);
	h_run("cible intel et ssbd", cible_intel_ssbd);
	h_run("cible eibrs et ibrs a l'entree", cible_eibrs_et_ibrs_entree);
	h_run("cible amd", cible_amd);
	h_run("cible bornee par les capacites",
		cible_ne_depasse_jamais_les_capacites);
	h_run("application ecriture et relecture",
		application_ecriture_et_relecture);
	h_run("application refusee", application_refusee_par_l_hyperviseur);
	return (h_end());
}
