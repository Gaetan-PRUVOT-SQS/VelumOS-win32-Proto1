#include "velum/boot.h"
#include "velum/klog.h"
#include "spec_apply.h"

uint8_t			g_spec_verw_user;
static uint16_t	g_spec_verw_sel = SPEC_VERW_SELECTOR;

static uint64_t	g_spec_ctrl_value;

static uint64_t	msr_read(uint32_t msr)
{
	uint32_t	lo;
	uint32_t	hi;

	__asm__ volatile ("rdmsr" : "=a"(lo), "=d"(hi) : "c"(msr));
	return (((uint64_t)hi << 32) | lo);
}

static void	msr_write(uint32_t msr, uint64_t value)
{
	__asm__ volatile ("wrmsr" : : "a"((uint32_t)value),
		"d"((uint32_t)(value >> 32)), "c"(msr) : "memory");
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

void	cpu_spec_apply_local(void)
{
	uint64_t	target;

	target = __atomic_load_n(&g_spec_ctrl_value, __ATOMIC_RELAXED);
	if (target)
		msr_write(MSR_SPEC_CTRL, target);
}

static void	spec_apply_ctrl(const t_cpuvuln *v, const t_specplan *plan)
{
	t_specapply	res;
	uint64_t	target;

	target = spec_msr_target(v, plan, !boot_cmdline_has("nossbd"));
	res = spec_msr_apply(msr_write, msr_read, target);
	if (res == SPEC_APPLY_OK)
		__atomic_store_n(&g_spec_ctrl_value, target, __ATOMIC_RELAXED);
	if (res == SPEC_APPLY_NONE)
		klog_info("cpu: spec_ctrl non écrit, rien à activer");
	if (res == SPEC_APPLY_OK)
		klog_info("cpu: spec_ctrl écrit %#x (ibrs %d stibp %d ssbd %d)",
			(unsigned)target, (int)(target & SPEC_CTRL_IBRS),
			(int)((target & SPEC_CTRL_STIBP) >> 1),
			(int)((target & SPEC_CTRL_SSBD) >> 2));
	if (res == SPEC_APPLY_REFUSED)
		klog_warn("cpu: spec_ctrl %#x refusé à la relecture, non appliqué",
			(unsigned)target);
}

static void	spec_announce(const t_cpuvuln *v, const t_specplan *plan)
{
	char	flags[200];

	cpu_vuln_format(v, flags, sizeof(flags));
	klog_info("cpu: mitigations specs, bits : %s", flags[0] ? flags : "aucun");
	klog_info("cpu: meltdown %s, kpti %s (pcid %s invpcid %s), kpti non appliqué",
		spec_state_label(plan->base.meltdown), plan->kpti.enabled ? "requis"
		: "inutile", onoff(plan->kpti.use_pcid), onoff(plan->kpti.use_invpcid));
	klog_info("cpu: mds %s, taa %s, mmio %s",
		spec_state_label(plan->base.mds), spec_state_label(plan->taa),
		spec_state_label(plan->mmio));
	klog_info("cpu: verw au retour utilisateur : %s",
		onoff(plan->verw_user_return));
	klog_info("cpu: spectre v2 prévu : %s, rsb %s, ibpb au changement %s "
		"(rsb et ibpb non appliqués)",
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
	spec_apply_ctrl(&v, &plan);
}
