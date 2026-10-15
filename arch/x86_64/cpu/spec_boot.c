#include "velum/klog.h"
#include "spec_policy.h"

uint8_t			g_spec_verw_user;
static uint16_t	g_spec_verw_sel = SPEC_VERW_SELECTOR;

static uint64_t	msr_read(uint32_t msr)
{
	uint32_t	lo;
	uint32_t	hi;

	__asm__ volatile ("rdmsr" : "=a"(lo), "=d"(hi) : "c"(msr));
	return (((uint64_t)hi << 32) | lo);
}

bool	cpu_spec_verw_active(void)
{
	return (__atomic_load_n(&g_spec_verw_user, __ATOMIC_RELAXED) != 0);
}

void	cpu_spec_user_return(void)
{
	if (!g_spec_verw_user)
		return ;
	__asm__ volatile ("verw %0" : : "m"(g_spec_verw_sel) : "cc", "memory");
}

static const char	*onoff(bool value)
{
	return (value ? "oui" : "non");
}

static void	spec_announce(const t_cpuvuln *v, const t_specplan *plan)
{
	char	flags[200];

	cpu_vuln_format(v, flags, sizeof(flags));
	klog_info("cpu: mitigations specs, bits : %s", flags[0] ? flags : "aucun");
	klog_info("cpu: meltdown %s, kpti %s (pcid %s invpcid %s), non appliqué",
		spec_state_label(plan->base.meltdown), plan->kpti.enabled ? "requis"
		: "inutile", onoff(plan->kpti.use_pcid), onoff(plan->kpti.use_invpcid));
	klog_info("cpu: mds %s, taa %s, mmio %s",
		spec_state_label(plan->base.mds), spec_state_label(plan->taa),
		spec_state_label(plan->mmio));
	klog_info("cpu: verw au retour utilisateur %s",
		onoff(plan->verw_user_return));
	klog_info("cpu: spectre v2 prévu : %s, rsb %s, ibpb au changement %s",
		spec_v2_name(plan->v2), onoff(plan->rsb_fill),
		onoff(plan->ibpb_on_switch));
	klog_info("cpu: ssb %s, stibp %s, spec_ctrl noyau %#x utilisateur %#x",
		spec_state_label(plan->ssb), onoff(plan->stibp_on),
		(unsigned)plan->spec_ctrl_kernel, (unsigned)plan->spec_ctrl_user);
	if (plan->base.mds == SPEC_OPEN)
		klog_warn("cpu: microcode sans md_clear, mds non atténué");
	if (plan->taa == SPEC_OPEN)
		klog_warn("cpu: rtm présent sans md_clear, taa non atténué");
	if (plan->mmio == SPEC_UNKNOWN)
		klog_warn("cpu: mmio stale data non évalué, table de modèles absente");
}

void	cpu_vuln_log(void)
{
	t_cpuvuln	v;
	t_specid	id;
	t_specplan	plan;

	cpu_vuln_decode(cpu_raw(), msr_read, &v);
	spec_ident_decode(cpu_raw(), spec_vendor_of(cpu_features()->vendor), &id);
	spec_plan_decide(&v, &id, &plan);
	__atomic_store_n(&g_spec_verw_user, plan.verw_user_return,
		__ATOMIC_RELAXED);
	spec_announce(&v, &plan);
}
