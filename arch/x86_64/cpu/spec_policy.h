#ifndef SPEC_POLICY_H
# define SPEC_POLICY_H

# include "cpu_vuln.h"

/* SDM vol.4 table 2-2, IA32_ARCH_CAPABILITIES (10AH) ; numéros de bit non relus en session, bits 13 à 15, 19 et 20 non vérifiés */
# define ARCHCAP_RDCL_NO 0
# define ARCHCAP_IBRS_ALL 1
# define ARCHCAP_RSBA 2
# define ARCHCAP_SKIP_L1DFL 3
# define ARCHCAP_SSB_NO 4
# define ARCHCAP_MDS_NO 5
# define ARCHCAP_PSCHANGE_MC_NO 6
# define ARCHCAP_TSX_CTRL 7
# define ARCHCAP_TAA_NO 8
# define ARCHCAP_SBDR_SSDP_NO 13
# define ARCHCAP_FBSDP_NO 14
# define ARCHCAP_PSDP_NO 15
# define ARCHCAP_FB_CLEAR 17
# define ARCHCAP_RRSBA 19
# define ARCHCAP_BHI_NO 20
# define SPEC_VERW_SELECTOR GDT_KDATA

typedef enum e_specvendor
{
	SPEC_VENDOR_OTHER,
	SPEC_VENDOR_INTEL,
	SPEC_VENDOR_AMD
}	t_specvendor;

typedef enum e_specstate
{
	SPEC_SAFE,
	SPEC_VERW,
	SPEC_OPEN,
	SPEC_UNKNOWN,
	SPEC_CTRL_AVAIL
}	t_specstate;

typedef struct s_specpolicy
{
	t_specstate	mds;
	t_specstate	meltdown;
	bool		verw_user_return;
	bool		ibrs_usable;
	bool		ibpb_usable;
	bool		stibp_usable;
	bool		ssbd_usable;
}	t_specpolicy;

t_specvendor	spec_vendor_of(const char *vendor);
void			spec_policy_decide(const t_cpuvuln *v, t_specvendor vendor,
					t_specpolicy *out);
const char		*spec_state_name(t_specstate state);

typedef enum e_specv2
{
	SPEC_V2_RETPOLINE,
	SPEC_V2_EIBRS,
	SPEC_V2_EIBRS_RETPOLINE,
	SPEC_V2_IBRS_ENTRY
}	t_specv2;

typedef struct s_specid
{
	t_specvendor	vendor;
	uint32_t		family;
	uint32_t		model;
	uint32_t		stepping;
	bool			rtm;
	bool			pcid;
	bool			invpcid;
	bool			htt;
	bool			amd_ssb_no;
}	t_specid;

typedef struct s_speckpti
{
	bool	enabled;
	bool	use_pcid;
	bool	use_invpcid;
}	t_speckpti;

typedef struct s_specplan
{
	t_specpolicy	base;
	t_specstate		taa;
	t_specstate		mmio;
	t_specstate		ssb;
	t_specv2		v2;
	t_speckpti		kpti;
	bool			eibrs;
	bool			rsb_fill;
	bool			ibpb_on_switch;
	bool			stibp_on;
	bool			verw_user_return;
	uint64_t		spec_ctrl_kernel;
	uint64_t		spec_ctrl_user;
}	t_specplan;

static inline bool	spec_archcap(const t_cpuvuln *v, int bit)
{
	return (v->arch_cap_read && ((v->arch_cap_value >> bit) & 1u));
}

static inline void	spec_ident_decode(const t_cpuid_raw *raw,
						t_specvendor vendor, t_specid *out)
{
	uint32_t	sig;
	uint32_t	base;

	sig = raw->r[CPUID_L1][R_EAX];
	base = (sig >> 8) & 0xf;
	out->vendor = vendor;
	out->family = base;
	out->model = (sig >> 4) & 0xf;
	out->stepping = sig & 0xf;
	if (base == 0xf)
		out->family += (sig >> 20) & 0xff;
	if (base == 0xf || (base == 0x6 && vendor == SPEC_VENDOR_INTEL))
		out->model |= ((sig >> 16) & 0xf) << 4;
	out->rtm = cpuid_bit(raw, CPUID_L7, R_EBX, CPUID7_EBX_RTM);
	out->invpcid = cpuid_bit(raw, CPUID_L7, R_EBX, CPUID7_EBX_INVPCID);
	out->pcid = cpuid_bit(raw, CPUID_L1, R_ECX, CPUID1_ECX_PCID);
	out->htt = cpuid_bit(raw, CPUID_L1, R_EDX, CPUID1_EDX_HTT);
	out->amd_ssb_no = cpuid_bit(raw, CPUID_E8, R_EBX, CPUIDE8_EBX_AMD_SSB_NO);
}

static inline t_specstate	spec_taa_state(const t_cpuvuln *v,
								const t_specid *id)
{
	if (id->vendor == SPEC_VENDOR_AMD || !id->rtm)
		return (SPEC_SAFE);
	if (spec_archcap(v, ARCHCAP_TAA_NO))
		return (SPEC_SAFE);
	if (id->vendor == SPEC_VENDOR_OTHER)
		return (SPEC_UNKNOWN);
	if (v->md_clear)
		return (SPEC_VERW);
	return (SPEC_OPEN);
}

static inline t_specstate	spec_mmio_state(const t_cpuvuln *v,
								const t_specid *id)
{
	if (id->vendor == SPEC_VENDOR_AMD)
		return (SPEC_SAFE);
	if (spec_archcap(v, ARCHCAP_SBDR_SSDP_NO)
		&& spec_archcap(v, ARCHCAP_FBSDP_NO)
		&& spec_archcap(v, ARCHCAP_PSDP_NO))
		return (SPEC_SAFE);
	return (SPEC_UNKNOWN);
}

static inline t_specstate	spec_ssb_state(const t_cpuvuln *v,
								const t_specid *id)
{
	if (spec_archcap(v, ARCHCAP_SSB_NO))
		return (SPEC_SAFE);
	if (id->vendor == SPEC_VENDOR_AMD && id->amd_ssb_no)
		return (SPEC_SAFE);
	if (v->ssbd || v->amd_ssbd)
		return (SPEC_CTRL_AVAIL);
	return (SPEC_OPEN);
}

static inline t_specv2	spec_v2_choice(const t_cpuvuln *v, const t_specid *id)
{
	bool	eibrs;

	eibrs = v->spec_ctrl && spec_archcap(v, ARCHCAP_IBRS_ALL);
	if (id->vendor != SPEC_VENDOR_INTEL)
		return (SPEC_V2_RETPOLINE);
	if (eibrs && spec_archcap(v, ARCHCAP_RRSBA))
		return (SPEC_V2_EIBRS_RETPOLINE);
	if (eibrs)
		return (SPEC_V2_EIBRS);
	if (v->spec_ctrl && spec_archcap(v, ARCHCAP_RSBA))
		return (SPEC_V2_IBRS_ENTRY);
	return (SPEC_V2_RETPOLINE);
}

static inline t_speckpti	spec_kpti_decide(t_specstate meltdown,
								const t_specid *id)
{
	t_speckpti	k;

	k.enabled = meltdown != SPEC_SAFE;
	k.use_pcid = k.enabled && id->pcid;
	k.use_invpcid = k.use_pcid && id->invpcid;
	return (k);
}

static inline void	spec_ctrl_values(t_specplan *plan, bool stibp_usable)
{
	uint64_t	kernel;
	uint64_t	user;

	kernel = 0;
	user = 0;
	if (plan->v2 == SPEC_V2_EIBRS || plan->v2 == SPEC_V2_EIBRS_RETPOLINE)
	{
		kernel |= SPEC_CTRL_IBRS;
		user |= SPEC_CTRL_IBRS;
	}
	if (plan->v2 == SPEC_V2_IBRS_ENTRY)
		kernel |= SPEC_CTRL_IBRS;
	if (plan->stibp_on && stibp_usable)
	{
		kernel |= SPEC_CTRL_STIBP;
		user |= SPEC_CTRL_STIBP;
	}
	plan->spec_ctrl_kernel = kernel;
	plan->spec_ctrl_user = user;
}

static inline void	spec_plan_decide(const t_cpuvuln *v, const t_specid *id,
						t_specplan *out)
{
	*out = (t_specplan){0};
	spec_policy_decide(v, id->vendor, &out->base);
	out->taa = spec_taa_state(v, id);
	out->mmio = spec_mmio_state(v, id);
	out->ssb = spec_ssb_state(v, id);
	out->v2 = spec_v2_choice(v, id);
	out->eibrs = out->v2 == SPEC_V2_EIBRS
		|| out->v2 == SPEC_V2_EIBRS_RETPOLINE;
	out->kpti = spec_kpti_decide(out->base.meltdown, id);
	out->rsb_fill = out->v2 != SPEC_V2_EIBRS;
	out->ibpb_on_switch = out->base.ibpb_usable;
	out->stibp_on = id->htt && out->base.stibp_usable && !out->eibrs;
	out->verw_user_return = out->base.verw_user_return
		|| out->taa == SPEC_VERW;
	spec_ctrl_values(out, out->base.stibp_usable);
}

static inline const char	*spec_state_label(t_specstate state)
{
	if (state == SPEC_CTRL_AVAIL)
		return ("contrôle disponible, non activé");
	return (spec_state_name(state));
}

static inline const char	*spec_v2_name(t_specv2 v2)
{
	if (v2 == SPEC_V2_EIBRS)
		return ("eibrs");
	if (v2 == SPEC_V2_EIBRS_RETPOLINE)
		return ("eibrs et retpoline");
	if (v2 == SPEC_V2_IBRS_ENTRY)
		return ("ibrs à l'entrée noyau");
	return ("retpoline");
}

#endif
