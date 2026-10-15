#include <string.h>
#include "harness.h"
#include "cpu_int.h"
#include "spec_policy.h"

typedef struct s_speccase
{
	const char		*name;
	t_specvendor	vendor;
	bool			md_clear;
	bool			arch_cap_read;
	uint64_t		arch_cap;
	t_specstate		mds;
	t_specstate		meltdown;
	bool			verw;
}	t_speccase;

#define B(n) (1ull << (n))

static const t_speccase	g_cases[] = {
{"D1 intel md_clear sans arch_cap", SPEC_VENDOR_INTEL, true, false, 0,
	SPEC_VERW, SPEC_OPEN, true},
{"D2 intel md_clear mds_no", SPEC_VENDOR_INTEL, true, true, B(5) | B(0),
	SPEC_SAFE, SPEC_SAFE, false},
{"D3 intel md_clear arch_cap sans mds_no", SPEC_VENDOR_INTEL, true, true,
	B(0), SPEC_VERW, SPEC_SAFE, true},
{"D4 intel sans md_clear", SPEC_VENDOR_INTEL, false, false, 0,
	SPEC_OPEN, SPEC_OPEN, false},
{"D5 intel sans md_clear mais mds_no", SPEC_VENDOR_INTEL, false, true, B(5),
	SPEC_SAFE, SPEC_OPEN, false},
{"D6 intel rdcl_no seul sans md_clear", SPEC_VENDOR_INTEL, false, true, B(0),
	SPEC_OPEN, SPEC_SAFE, false},
{"D7 amd md_clear", SPEC_VENDOR_AMD, true, false, 0,
	SPEC_SAFE, SPEC_SAFE, false},
{"D8 amd sans rien", SPEC_VENDOR_AMD, false, false, 0,
	SPEC_SAFE, SPEC_SAFE, false},
{"D9 inconnu md_clear", SPEC_VENDOR_OTHER, true, false, 0,
	SPEC_VERW, SPEC_UNKNOWN, true},
{"D10 inconnu sans md_clear", SPEC_VENDOR_OTHER, false, false, 0,
	SPEC_UNKNOWN, SPEC_UNKNOWN, false},
{"D11 inconnu mds_no", SPEC_VENDOR_OTHER, true, true, B(5),
	SPEC_SAFE, SPEC_UNKNOWN, false},
{"D12 valeur arch_cap ignoree si non lue", SPEC_VENDOR_INTEL, true, false,
	B(5) | B(0), SPEC_VERW, SPEC_OPEN, true},
{NULL, SPEC_VENDOR_OTHER, false, false, 0, SPEC_SAFE, SPEC_SAFE, false}
};

static uint64_t	g_msr_value;
static int		g_msr_reads;
static uint32_t	g_msr_last;

static uint64_t	fake_msr(uint32_t msr)
{
	g_msr_reads++;
	g_msr_last = msr;
	return (g_msr_value);
}

static void	table_decision(void)
{
	t_specpolicy	p;
	t_cpuvuln		v;
	int				i;

	i = 0;
	while (g_cases[i].name)
	{
		memset(&v, 0, sizeof(v));
		v.md_clear = g_cases[i].md_clear;
		v.arch_cap_read = g_cases[i].arch_cap_read;
		v.arch_cap_value = g_cases[i].arch_cap;
		spec_policy_decide(&v, g_cases[i].vendor, &p);
		h_eq_i64(g_cases[i].name, p.mds, g_cases[i].mds);
		h_eq_i64(g_cases[i].name, p.meltdown, g_cases[i].meltdown);
		h_eq_u64(g_cases[i].name, p.verw_user_return, g_cases[i].verw);
		i++;
	}
}

static void	decode_bits_cpuid(void)
{
	t_cpuid_raw	raw;
	t_cpuvuln	v;

	memset(&raw, 0, sizeof(raw));
	cpu_vuln_decode(&raw, fake_msr, &v);
	h_true(!v.md_clear && !v.spec_ctrl && !v.stibp && !v.l1d_flush
		&& !v.arch_cap && !v.ssbd && !v.amd_ibpb && !v.amd_ibrs
		&& !v.amd_stibp && !v.amd_ssbd && !v.arch_cap_read,
		"cpuid vide, aucun bit");
	raw.r[CPUID_L7][R_EDX] = B(10);
	cpu_vuln_decode(&raw, fake_msr, &v);
	h_true(v.md_clear && !v.spec_ctrl, "edx bit 10 md_clear seul");
	raw.r[CPUID_L7][R_EDX] = B(26);
	cpu_vuln_decode(&raw, fake_msr, &v);
	h_true(v.spec_ctrl && !v.md_clear && !v.stibp, "edx bit 26 spec_ctrl");
	raw.r[CPUID_L7][R_EDX] = B(27) | B(28) | B(31);
	cpu_vuln_decode(&raw, fake_msr, &v);
	h_true(v.stibp && v.l1d_flush && v.ssbd && !v.arch_cap,
		"edx bits 27 28 31");
	raw.r[CPUID_L7][R_EDX] = 0;
	raw.r[CPUID_E8][R_EBX] = B(12) | B(14) | B(15) | B(24);
	cpu_vuln_decode(&raw, fake_msr, &v);
	h_true(v.amd_ibpb && v.amd_ibrs && v.amd_stibp && v.amd_ssbd
		&& !v.spec_ctrl && !v.ssbd, "ebx 8000_0008 bits 12 14 15 24");
	raw.r[CPUID_E8][R_EBX] = B(13) | B(16) | B(25);
	cpu_vuln_decode(&raw, fake_msr, &v);
	h_true(!v.amd_ibpb && !v.amd_ibrs && !v.amd_stibp && !v.amd_ssbd,
		"ebx 8000_0008 bits voisins ignores");
}

static void	lecture_msr_conditionnelle(void)
{
	t_cpuid_raw	raw;
	t_cpuvuln	v;

	memset(&raw, 0, sizeof(raw));
	g_msr_value = B(5);
	g_msr_reads = 0;
	cpu_vuln_decode(&raw, fake_msr, &v);
	h_eq_i64("msr non lu sans bit arch_cap", g_msr_reads, 0);
	h_true(!v.arch_cap_read && v.arch_cap_value == 0, "valeur nulle");
	raw.r[CPUID_L7][R_EDX] = B(29);
	cpu_vuln_decode(&raw, NULL, &v);
	h_true(v.arch_cap && !v.arch_cap_read, "sans lecteur, pas de lecture");
	cpu_vuln_decode(&raw, fake_msr, &v);
	h_eq_i64("msr lu une fois", g_msr_reads, 1);
	h_eq_u64("numero msr 0x10a", g_msr_last, 0x10a);
	h_true(v.arch_cap_read && v.arch_cap_value == B(5), "valeur lue");
}

static void	format_et_tables(void)
{
	t_cpuvuln	v;
	char		b[200];
	char		small[8];

	memset(&v, 0, sizeof(v));
	cpu_vuln_format(&v, b, sizeof(b));
	h_eq_str("format vide", b, "");
	v.md_clear = true;
	v.arch_cap = true;
	v.arch_cap_read = true;
	v.arch_cap_value = B(0) | B(5);
	cpu_vuln_format(&v, b, sizeof(b));
	h_eq_str("format md_clear et arch_cap", b,
		"md_clear arch_cap rdcl_no mds_no");
	v.arch_cap_read = false;
	cpu_vuln_format(&v, b, sizeof(b));
	h_eq_str("format arch_cap non lu", b, "md_clear arch_cap");
	memset(small, 'x', sizeof(small));
	cpu_vuln_format(&v, small, 0);
	h_eq_u64("taille nulle sans ecriture", (uint8_t)small[0], 'x');
	cpu_vuln_format(&v, small, sizeof(small));
	h_eq_str("troncature", small, "md_clea");
	h_eq_str("vendeur intel", spec_vendor_of("GenuineIntel") == SPEC_VENDOR_INTEL
		? "ok" : "ko", "ok");
	h_true(spec_vendor_of("AuthenticAMD") == SPEC_VENDOR_AMD
		&& spec_vendor_of("HygonGenuine") == SPEC_VENDOR_AMD
		&& spec_vendor_of("TCGTCGTCGTCG") == SPEC_VENDOR_OTHER
		&& spec_vendor_of("") == SPEC_VENDOR_OTHER
		&& spec_vendor_of(NULL) == SPEC_VENDOR_OTHER, "vendeurs");
}

static void	usages_controles(void)
{
	t_specpolicy	p;
	t_cpuvuln		v;

	memset(&v, 0, sizeof(v));
	spec_policy_decide(&v, SPEC_VENDOR_INTEL, &p);
	h_true(!p.ibrs_usable && !p.ibpb_usable && !p.stibp_usable
		&& !p.ssbd_usable, "aucun controle");
	v.spec_ctrl = true;
	spec_policy_decide(&v, SPEC_VENDOR_INTEL, &p);
	h_true(p.ibrs_usable && p.ibpb_usable && !p.stibp_usable
		&& !p.ssbd_usable, "spec_ctrl donne ibrs et ibpb");
	memset(&v, 0, sizeof(v));
	v.amd_stibp = true;
	v.amd_ssbd = true;
	spec_policy_decide(&v, SPEC_VENDOR_AMD, &p);
	h_true(!p.ibrs_usable && p.stibp_usable && p.ssbd_usable,
		"bits amd stibp et ssbd");
}

static void	ident_famille_modele(void)
{
	t_cpuid_raw	raw;
	t_specid	id;

	memset(&raw, 0, sizeof(raw));
	raw.r[CPUID_L1][R_EAX] = 0x000306a9;
	spec_ident_decode(&raw, SPEC_VENDOR_INTEL, &id);
	h_eq_u64("intel famille 6", id.family, 6);
	h_eq_u64("intel modele etendu 0x3a", id.model, 0x3a);
	h_eq_u64("intel pas 9", id.stepping, 9);
	spec_ident_decode(&raw, SPEC_VENDOR_AMD, &id);
	h_eq_u64("amd famille 6 sans modele etendu", id.model, 0xa);
	raw.r[CPUID_L1][R_EAX] = 0x00a20f10;
	spec_ident_decode(&raw, SPEC_VENDOR_AMD, &id);
	h_eq_u64("amd famille 0xf plus etendue 0xa", id.family, 0x19);
	h_eq_u64("amd modele etendu 0x21", id.model, 0x21);
	raw.r[CPUID_L1][R_EAX] = 0x000506e3;
	spec_ident_decode(&raw, SPEC_VENDOR_INTEL, &id);
	h_eq_u64("intel skylake modele 0x5e", id.model, 0x5e);
	h_true(!id.rtm && !id.pcid && !id.invpcid && !id.htt,
		"aucun drapeau sans bit cpuid");
	raw.r[CPUID_L7][R_EBX] = B(10) | B(11);
	raw.r[CPUID_L1][R_ECX] = B(17);
	raw.r[CPUID_L1][R_EDX] = B(28);
	raw.r[CPUID_E8][R_EBX] = B(26);
	spec_ident_decode(&raw, SPEC_VENDOR_INTEL, &id);
	h_true(id.rtm && id.pcid && id.invpcid && id.htt && id.amd_ssb_no,
		"drapeaux rtm pcid invpcid htt ssb_no");
}

static void	kpti_cas_limites(void)
{
	t_specid	id;
	t_speckpti	k;

	memset(&id, 0, sizeof(id));
	k = spec_kpti_decide(SPEC_SAFE, &id);
	h_true(!k.enabled && !k.use_pcid && !k.use_invpcid, "kpti inutile");
	id.pcid = true;
	id.invpcid = true;
	k = spec_kpti_decide(SPEC_SAFE, &id);
	h_true(!k.enabled && !k.use_pcid && !k.use_invpcid,
		"sain, pcid ignore");
	k = spec_kpti_decide(SPEC_OPEN, &id);
	h_true(k.enabled && k.use_pcid && k.use_invpcid, "ouvert pcid invpcid");
	k = spec_kpti_decide(SPEC_UNKNOWN, &id);
	h_true(k.enabled, "inconnu traite comme vulnerable");
	id.pcid = false;
	k = spec_kpti_decide(SPEC_OPEN, &id);
	h_true(k.enabled && !k.use_pcid && !k.use_invpcid,
		"invpcid sans pcid inutilisable");
	id.pcid = true;
	id.invpcid = false;
	k = spec_kpti_decide(SPEC_OPEN, &id);
	h_true(k.use_pcid && !k.use_invpcid, "pcid sans invpcid");
}

static void	plan_intel_skylake_sans_arch_cap(void)
{
	t_specplan	plan;
	t_cpuvuln	v;
	t_specid	id;

	memset(&v, 0, sizeof(v));
	memset(&id, 0, sizeof(id));
	id.vendor = SPEC_VENDOR_INTEL;
	id.pcid = true;
	id.htt = true;
	v.spec_ctrl = true;
	v.stibp = true;
	v.ssbd = true;
	v.md_clear = true;
	spec_plan_decide(&v, &id, &plan);
	h_true(plan.kpti.enabled && plan.kpti.use_pcid, "kpti requis");
	h_eq_i64("spectre v2 retpoline sans rsba", plan.v2, SPEC_V2_RETPOLINE);
	h_true(plan.verw_user_return && plan.rsb_fill && plan.ibpb_on_switch,
		"verw rsb ibpb");
	h_true(plan.stibp_on, "stibp avec smt et sans eibrs");
	h_eq_u64("spec_ctrl noyau stibp", plan.spec_ctrl_kernel, 2);
	h_eq_u64("spec_ctrl utilisateur stibp", plan.spec_ctrl_user, 2);
	h_eq_i64("ssb controle disponible", plan.ssb, SPEC_CTRL_AVAIL);
	h_eq_i64("mmio inconnu sans bits no", plan.mmio, SPEC_UNKNOWN);
	h_eq_i64("taa sain sans rtm", plan.taa, SPEC_SAFE);
}

static void	plan_variantes_spectre_v2(void)
{
	t_specplan	plan;
	t_cpuvuln	v;
	t_specid	id;

	memset(&v, 0, sizeof(v));
	memset(&id, 0, sizeof(id));
	id.vendor = SPEC_VENDOR_INTEL;
	v.spec_ctrl = true;
	v.arch_cap = true;
	v.arch_cap_read = true;
	v.arch_cap_value = B(2);
	spec_plan_decide(&v, &id, &plan);
	h_eq_i64("rsba avec spec_ctrl, ibrs a l'entree", plan.v2,
		SPEC_V2_IBRS_ENTRY);
	h_eq_u64("ibrs noyau seulement", plan.spec_ctrl_kernel, 1);
	h_eq_u64("ibrs absent cote utilisateur", plan.spec_ctrl_user, 0);
	v.spec_ctrl = false;
	spec_plan_decide(&v, &id, &plan);
	h_eq_i64("rsba sans spec_ctrl, retpoline", plan.v2, SPEC_V2_RETPOLINE);
	v.spec_ctrl = true;
	v.arch_cap_value = B(1) | B(2);
	spec_plan_decide(&v, &id, &plan);
	h_eq_i64("eibrs prime sur rsba", plan.v2, SPEC_V2_EIBRS);
	h_true(plan.eibrs && !plan.rsb_fill, "eibrs sans remplissage rsb");
	h_eq_u64("ibrs permanent", plan.spec_ctrl_user, 1);
	v.arch_cap_value = B(1) | B(19);
	spec_plan_decide(&v, &id, &plan);
	h_eq_i64("eibrs avec rrsba", plan.v2, SPEC_V2_EIBRS_RETPOLINE);
	v.spec_ctrl = false;
	spec_plan_decide(&v, &id, &plan);
	h_eq_i64("ibrs_all sans spec_ctrl cpuid ignore", plan.v2,
		SPEC_V2_RETPOLINE);
	v.arch_cap_read = false;
	v.spec_ctrl = true;
	spec_plan_decide(&v, &id, &plan);
	h_eq_i64("arch_cap non lu, retpoline", plan.v2, SPEC_V2_RETPOLINE);
	id.vendor = SPEC_VENDOR_AMD;
	v.arch_cap_read = true;
	v.arch_cap_value = B(1);
	spec_plan_decide(&v, &id, &plan);
	h_eq_i64("amd retpoline", plan.v2, SPEC_V2_RETPOLINE);
}

static void	plan_taa_ssb_mmio(void)
{
	t_specplan	plan;
	t_cpuvuln	v;
	t_specid	id;

	memset(&v, 0, sizeof(v));
	memset(&id, 0, sizeof(id));
	id.vendor = SPEC_VENDOR_INTEL;
	id.rtm = true;
	v.md_clear = true;
	v.arch_cap = true;
	v.arch_cap_read = true;
	v.arch_cap_value = B(5) | B(0);
	spec_plan_decide(&v, &id, &plan);
	h_eq_i64("mds sain", plan.base.mds, SPEC_SAFE);
	h_eq_i64("taa verw malgre mds_no", plan.taa, SPEC_VERW);
	h_true(plan.verw_user_return && !plan.base.verw_user_return,
		"verw tire par taa seul");
	v.arch_cap_value |= B(8);
	spec_plan_decide(&v, &id, &plan);
	h_true(plan.taa == SPEC_SAFE && !plan.verw_user_return, "taa_no");
	v.arch_cap_value = B(5) | B(0);
	v.md_clear = false;
	spec_plan_decide(&v, &id, &plan);
	h_eq_i64("taa ouvert sans md_clear", plan.taa, SPEC_OPEN);
	id.vendor = SPEC_VENDOR_OTHER;
	spec_plan_decide(&v, &id, &plan);
	h_eq_i64("taa inconnu hors intel et amd", plan.taa, SPEC_UNKNOWN);
	id.vendor = SPEC_VENDOR_AMD;
	spec_plan_decide(&v, &id, &plan);
	h_eq_i64("taa amd sain", plan.taa, SPEC_SAFE);
	id.vendor = SPEC_VENDOR_INTEL;
	v.arch_cap_value = B(13) | B(14) | B(15) | B(4);
	spec_plan_decide(&v, &id, &plan);
	h_eq_i64("mmio sain avec trois bits no", plan.mmio, SPEC_SAFE);
	h_eq_i64("ssb_no", plan.ssb, SPEC_SAFE);
	v.arch_cap_value = B(13) | B(14);
	spec_plan_decide(&v, &id, &plan);
	h_eq_i64("mmio inconnu avec deux bits no", plan.mmio, SPEC_UNKNOWN);
	h_eq_i64("ssb ouvert sans ssbd", plan.ssb, SPEC_OPEN);
	id.vendor = SPEC_VENDOR_AMD;
	id.amd_ssb_no = true;
	spec_plan_decide(&v, &id, &plan);
	h_eq_i64("ssb_no amd", plan.ssb, SPEC_SAFE);
}

static void	noms_et_stibp(void)
{
	t_specplan	plan;
	t_cpuvuln	v;
	t_specid	id;

	h_eq_str("nom controle disponible",
		spec_state_label(SPEC_CTRL_AVAIL), "contrôle disponible, non activé");
	h_eq_str("nom delegue", spec_state_label(SPEC_SAFE), "non affecté");
	h_eq_str("nom v2", spec_v2_name(SPEC_V2_IBRS_ENTRY),
		"ibrs à l'entrée noyau");
	memset(&v, 0, sizeof(v));
	memset(&id, 0, sizeof(id));
	id.vendor = SPEC_VENDOR_AMD;
	v.amd_stibp = true;
	spec_plan_decide(&v, &id, &plan);
	h_true(!plan.stibp_on && plan.spec_ctrl_kernel == 0,
		"stibp inactif sans htt");
	id.htt = true;
	spec_plan_decide(&v, &id, &plan);
	h_true(plan.stibp_on && plan.spec_ctrl_kernel == 2, "stibp amd avec htt");
	h_true(!plan.ibpb_on_switch, "pas d'ibpb sans controle");
	id.vendor = SPEC_VENDOR_INTEL;
	v.amd_stibp = false;
	v.stibp = true;
	v.spec_ctrl = true;
	v.arch_cap = true;
	v.arch_cap_read = true;
	v.arch_cap_value = B(1);
	spec_plan_decide(&v, &id, &plan);
	h_true(plan.eibrs && !plan.stibp_on && plan.spec_ctrl_kernel == 1,
		"eibrs sans stibp logiciel");
}

int	main(void)
{
	h_begin("a01/spec");
	h_run("table de decision", table_decision);
	h_run("bits cpuid", decode_bits_cpuid);
	h_run("lecture msr conditionnelle", lecture_msr_conditionnelle);
	h_run("format et vendeurs", format_et_tables);
	h_run("controles utilisables", usages_controles);
	h_run("famille et modele", ident_famille_modele);
	h_run("kpti cas limites", kpti_cas_limites);
	h_run("plan intel sans arch_cap", plan_intel_skylake_sans_arch_cap);
	h_run("variantes spectre v2", plan_variantes_spectre_v2);
	h_run("taa ssb mmio", plan_taa_ssb_mmio);
	h_run("noms et stibp", noms_et_stibp);
	return (h_end());
}
