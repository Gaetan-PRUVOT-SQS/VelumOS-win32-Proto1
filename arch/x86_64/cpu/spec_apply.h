#ifndef SPEC_APPLY_H
# define SPEC_APPLY_H

# include "spec_policy.h"

typedef void	(*t_msrwritefn)(uint32_t msr, uint64_t value);

typedef enum e_specapply
{
	SPEC_APPLY_NONE,
	SPEC_APPLY_OK,
	SPEC_APPLY_REFUSED
}	t_specapply;

static inline uint64_t	spec_msr_mask(const t_cpuvuln *v)
{
	uint64_t	mask;

	mask = 0;
	if (v->spec_ctrl || v->amd_ibrs)
		mask |= SPEC_CTRL_IBRS;
	if (v->stibp || v->amd_stibp)
		mask |= SPEC_CTRL_STIBP;
	if (v->ssbd || v->amd_ssbd)
		mask |= SPEC_CTRL_SSBD;
	return (mask);
}

static inline uint64_t	spec_msr_target(const t_cpuvuln *v,
							const t_specplan *plan, bool ssbd_wanted)
{
	uint64_t	value;

	value = plan->spec_ctrl_kernel;
	if (ssbd_wanted && plan->ssb == SPEC_CTRL_AVAIL)
		value |= SPEC_CTRL_SSBD;
	return (value & spec_msr_mask(v));
}

static inline t_specapply	spec_msr_apply(t_msrwritefn wr, t_msrreadfn rd,
								uint64_t target)
{
	if (!target)
		return (SPEC_APPLY_NONE);
	wr(MSR_SPEC_CTRL, target);
	if (rd(MSR_SPEC_CTRL) != target)
		return (SPEC_APPLY_REFUSED);
	return (SPEC_APPLY_OK);
}

#endif
