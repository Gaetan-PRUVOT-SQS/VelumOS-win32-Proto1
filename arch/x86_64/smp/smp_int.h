#ifndef SMP_INT_H
# define SMP_INT_H

# define TRAMP_PAGES 4
# define TRAMP_LOW_LIMIT 0x100000
# define TRAMP_DATA 0x800
# define TD_GDTR 0x800
# define TD_FAR32 0x808
# define TD_FAR64 0x810
# define TD_CR3 0x818
# define TD_STACK 0x820
# define TD_ENTRY 0x828
# define TD_ARG 0x830
# define TD_GDT 0x840
# define TRAMP_SEL_CODE64 0x08
# define TRAMP_SEL_DATA 0x10
# define TRAMP_SEL_CODE32 0x18
# define TRAMP_GDT_ENTRIES 4

# ifndef __ASSEMBLER__

#  include <stdbool.h>
#  include <stddef.h>
#  include <stdint.h>
#  include "../cpu/cpu_int.h"

#  define TRAMP_PTE_PRESENT_RW 0x3ull
#  define TRAMP_PDE_HUGE_RW 0x83ull
#  define TRAMP_PML4_ENTRIES 512
#  define TRAMP_KERNEL_FIRST 256
#  define TRAMP_GDT_CODE64 0x00af9a000000ffffull
#  define TRAMP_GDT_DATA 0x00cf92000000ffffull
#  define TRAMP_GDT_CODE32 0x00cf9a000000ffffull
#  define SMP_START_TIMEOUT_NS 200000000ull
#  define SMP_INIT_DELAY_NS 10000000ull
#  define SMP_SIPI_DELAY_NS 200000ull
#  define SMP_IPI_TIMEOUT_NS 200000000ull
#  define SMP_AP_STACK_PAGES 8
#  define SMP_AP_IST_PAGES 12

typedef struct s_tramp_cfg
{
	uint64_t	base;
	uint64_t	pml4_phys;
	uint64_t	pdpt_phys;
	uint64_t	pd_phys;
	uint32_t	off_pm32;
	uint32_t	off_lm64;
	uint64_t	entry;
}	t_tramp_cfg;

typedef struct s_plan_in
{
	const uint32_t	*ids;
	uint32_t		count;
	uint32_t		bsp;
	uint32_t		want;
	bool			x2;
}	t_plan_in;

typedef struct s_smp
{
	t_cpu		cpu[CPU_MAX];
	t_gdt		gdt[CPU_MAX];
	t_tss		tss[CPU_MAX];
	uint32_t	apic[CPU_MAX];
	uint64_t	ipi[CPU_MAX];
	uint64_t	stack_top[CPU_MAX];
	uint8_t		*ist[CPU_MAX];
	t_dtr		idtr;
	uint64_t	cr3;
	uint32_t	online;
	uint32_t	started;
	bool		inited;
}	t_smp;

extern t_smp	g_smp;

extern const char	smp_tramp_start[];
extern const char	smp_tramp_pm32[];
extern const char	smp_tramp_lm64[];
extern const char	smp_tramp_end[];

int			smp_parse_count(const char *text, uint32_t *out);
uint32_t	smp_plan(const t_plan_in *in, uint32_t *ap_out, uint32_t cap);
void		smp_tramp_data(uint8_t *page, const t_tramp_cfg *cfg);
void		smp_tramp_tables(uint64_t *pml4, uint64_t *pdpt, uint64_t *pd,
				const uint64_t *kernel_pml4, const t_tramp_cfg *cfg);
void		smp_tramp_target(uint8_t *page, uint64_t stack_top, uint64_t arg);
_Noreturn void	smp_ap_main(uint64_t idx);
void		smp_ipi_install(void);

# endif
#endif
