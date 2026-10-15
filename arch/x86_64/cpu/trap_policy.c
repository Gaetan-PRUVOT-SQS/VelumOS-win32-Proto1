#include "velum/libk.h"
#include "cpu_int.h"
#include "spec_policy.h"

t_trapact	trap_classify(const t_trapin *in)
{
	bool	exception;

	exception = in->vec < EXC_COUNT;
	if (exception && !in->user && in->nested)
		return (TRAP_NESTED);
	if (in->handler)
		return (TRAP_HANDLER);
	if (!exception)
		return (TRAP_UNHANDLED);
	if (in->user && in->hook)
		return (TRAP_USER_HOOK);
	if (in->user)
		return (TRAP_USER_PANIC);
	if (in->vec == 3)
		return (TRAP_BREAKPOINT);
	return (TRAP_KERNEL_PANIC);
}

uint8_t	idt_vec_ist(uint32_t vec)
{
	if (vec == 8)
		return (IST_DOUBLE_FAULT);
	if (vec == 2)
		return (IST_NMI);
	if (vec == 18)
		return (IST_MCE);
	return (0);
}

uint8_t	idt_vec_dpl(uint32_t vec)
{
	if (vec == 3)
		return (3);
	return (0);
}

uint64_t	cr0_wanted(uint64_t cr0)
{
	cr0 |= CR0_WP | CR0_NE | CR0_MP;
	cr0 &= ~(CR0_EM | CR0_TS);
	return (cr0);
}

uint64_t	cr4_wanted(uint64_t cr4, const t_cpufeat *feat)
{
	cr4 |= CR4_OSFXSR | CR4_OSXMMEXCPT;
	if (feat->xsave)
		cr4 |= CR4_OSXSAVE;
	if (feat->pge)
		cr4 |= CR4_PGE;
	if (feat->smep)
		cr4 |= CR4_SMEP;
	if (feat->smap)
		cr4 |= CR4_SMAP;
	if (feat->umip)
		cr4 |= CR4_UMIP;
	if (feat->fsgsbase)
		cr4 |= CR4_FSGSBASE;
	return (cr4);
}

static const t_archcapbit	g_archcap_bits[] = {
{"rdcl_no", ARCHCAP_RDCL_NO},
{"ibrs_all", ARCHCAP_IBRS_ALL},
{"rsba", ARCHCAP_RSBA},
{"skip_l1dfl", ARCHCAP_SKIP_L1DFL},
{"ssb_no", ARCHCAP_SSB_NO},
{"mds_no", ARCHCAP_MDS_NO},
{"pschange_mc_no", ARCHCAP_PSCHANGE_MC_NO},
{"tsx_ctrl", ARCHCAP_TSX_CTRL},
{"taa_no", ARCHCAP_TAA_NO},
{"fb_clear", ARCHCAP_FB_CLEAR},
{NULL, 0}
};

const t_archcapbit	*cpu_vuln_archcap_bits(void)
{
	return (g_archcap_bits);
}

void	cpu_vuln_decode(const t_cpuid_raw *raw, t_msrreadfn rd, t_cpuvuln *out)
{
	memset(out, 0, sizeof(*out));
	out->md_clear = cpuid_bit(raw, CPUID_L7, R_EDX, CPUID7_EDX_MD_CLEAR);
	out->spec_ctrl = cpuid_bit(raw, CPUID_L7, R_EDX, CPUID7_EDX_SPEC_CTRL);
	out->stibp = cpuid_bit(raw, CPUID_L7, R_EDX, CPUID7_EDX_STIBP);
	out->l1d_flush = cpuid_bit(raw, CPUID_L7, R_EDX, CPUID7_EDX_L1D_FLUSH);
	out->arch_cap = cpuid_bit(raw, CPUID_L7, R_EDX, CPUID7_EDX_ARCH_CAP);
	out->ssbd = cpuid_bit(raw, CPUID_L7, R_EDX, CPUID7_EDX_SSBD);
	out->amd_ibpb = cpuid_bit(raw, CPUID_E8, R_EBX, CPUIDE8_EBX_IBPB);
	out->amd_ibrs = cpuid_bit(raw, CPUID_E8, R_EBX, CPUIDE8_EBX_IBRS);
	out->amd_stibp = cpuid_bit(raw, CPUID_E8, R_EBX, CPUIDE8_EBX_STIBP);
	out->amd_ssbd = cpuid_bit(raw, CPUID_E8, R_EBX, CPUIDE8_EBX_SSBD);
	if (!out->arch_cap || !rd)
		return ;
	out->arch_cap_value = rd(MSR_ARCH_CAPABILITIES);
	out->arch_cap_read = true;
}

static void	format_flag(char *buf, size_t size, bool on, const char *name)
{
	if (!on)
		return ;
	if (buf[0])
		strlcat(buf, " ", size);
	strlcat(buf, name, size);
}

void	cpu_vuln_format(const t_cpuvuln *v, char *buf, size_t size)
{
	const t_archcapbit	*b;

	if (!size)
		return ;
	buf[0] = '\0';
	format_flag(buf, size, v->spec_ctrl, "ibrs_ibpb");
	format_flag(buf, size, v->stibp, "stibp");
	format_flag(buf, size, v->ssbd, "ssbd");
	format_flag(buf, size, v->md_clear, "md_clear");
	format_flag(buf, size, v->l1d_flush, "l1d_flush");
	format_flag(buf, size, v->arch_cap, "arch_cap");
	format_flag(buf, size, v->amd_ibpb, "amd_ibpb");
	format_flag(buf, size, v->amd_ibrs, "amd_ibrs");
	format_flag(buf, size, v->amd_stibp, "amd_stibp");
	format_flag(buf, size, v->amd_ssbd, "amd_ssbd");
	b = g_archcap_bits;
	while (b->name)
	{
		format_flag(buf, size, v->arch_cap_read
			&& ((v->arch_cap_value >> b->bit) & 1u), b->name);
		b++;
	}
}

t_specvendor	spec_vendor_of(const char *vendor)
{
	if (!vendor)
		return (SPEC_VENDOR_OTHER);
	if (!strcmp(vendor, "GenuineIntel"))
		return (SPEC_VENDOR_INTEL);
	if (!strcmp(vendor, "AuthenticAMD") || !strcmp(vendor, "HygonGenuine"))
		return (SPEC_VENDOR_AMD);
	return (SPEC_VENDOR_OTHER);
}

static bool	archcap_has(const t_cpuvuln *v, int bit)
{
	return (v->arch_cap_read && ((v->arch_cap_value >> bit) & 1u));
}

static t_specstate	mds_state(const t_cpuvuln *v, t_specvendor vendor)
{
	if (vendor == SPEC_VENDOR_AMD || archcap_has(v, ARCHCAP_MDS_NO))
		return (SPEC_SAFE);
	if (v->md_clear)
		return (SPEC_VERW);
	if (vendor == SPEC_VENDOR_INTEL)
		return (SPEC_OPEN);
	return (SPEC_UNKNOWN);
}

static t_specstate	meltdown_state(const t_cpuvuln *v, t_specvendor vendor)
{
	if (vendor == SPEC_VENDOR_AMD || archcap_has(v, ARCHCAP_RDCL_NO))
		return (SPEC_SAFE);
	if (vendor == SPEC_VENDOR_INTEL)
		return (SPEC_OPEN);
	return (SPEC_UNKNOWN);
}

void	spec_policy_decide(const t_cpuvuln *v, t_specvendor vendor,
			t_specpolicy *out)
{
	memset(out, 0, sizeof(*out));
	out->mds = mds_state(v, vendor);
	out->meltdown = meltdown_state(v, vendor);
	out->verw_user_return = out->mds == SPEC_VERW;
	out->ibrs_usable = v->spec_ctrl || v->amd_ibrs;
	out->ibpb_usable = v->spec_ctrl || v->amd_ibpb;
	out->stibp_usable = v->stibp || v->amd_stibp;
	out->ssbd_usable = v->ssbd || v->amd_ssbd;
}

const char	*spec_state_name(t_specstate state)
{
	if (state == SPEC_SAFE)
		return ("non affecté");
	if (state == SPEC_VERW)
		return ("atténué par verw");
	if (state == SPEC_OPEN)
		return ("vulnérable, rien d'appliqué");
	return ("inconnu");
}
