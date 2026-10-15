#ifndef CPU_VULN_H
# define CPU_VULN_H

# include <stdbool.h>
# include <stddef.h>
# include <stdint.h>
# include "cpu_int.h"

# define RFLAGS_AC 0x40000ull
# define PROBE_FAULT 0x100000000ull
# define PROBE_USER_MIN 0x1000ull
# define PROBE_USER_END 0x0000800000000000ull
# define AUDIT_LOG_MAX 8
# define MSR_ARCH_CAPABILITIES 0x10a
# define CPUID7_EDX_MD_CLEAR 10
# define CPUID7_EDX_SPEC_CTRL 26
# define CPUID7_EDX_STIBP 27
# define CPUID7_EDX_L1D_FLUSH 28
# define CPUID7_EDX_ARCH_CAP 29
# define CPUID7_EDX_SSBD 31
# define CPUIDE8_EBX_IBPB 12
# define CPUIDE8_EBX_IBRS 14
# define CPUIDE8_EBX_STIBP 15
# define CPUIDE8_EBX_SSBD 24

typedef uint64_t	(*t_msrreadfn)(uint32_t msr);

typedef struct s_cpuvuln
{
	bool		md_clear;
	bool		spec_ctrl;
	bool		stibp;
	bool		l1d_flush;
	bool		arch_cap;
	bool		ssbd;
	bool		amd_ibpb;
	bool		amd_ibrs;
	bool		amd_stibp;
	bool		amd_ssbd;
	bool		arch_cap_read;
	uint64_t	arch_cap_value;
}	t_cpuvuln;

typedef struct s_archcapbit
{
	const char	*name;
	uint8_t		bit;
}	t_archcapbit;

void				cpu_vuln_decode(const t_cpuid_raw *raw, t_msrreadfn rd,
						t_cpuvuln *out);
void				cpu_vuln_format(const t_cpuvuln *v, char *buf,
						size_t size);
const t_archcapbit	*cpu_vuln_archcap_bits(void);
void				cpu_vuln_log(void);

uint64_t			cpu_probe_user_read(uint64_t addr);
void				cpu_entry_audit(const t_regs *regs);
void				cpu_entry_audit_enable(void);
bool				cpu_entry_audit_enabled(void);
void				cpu_hard_boot(void);

#endif
