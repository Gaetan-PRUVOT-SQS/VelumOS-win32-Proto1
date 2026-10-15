#ifndef CPU_INT_H
# define CPU_INT_H

# include <stdbool.h>
# include <stddef.h>
# include <stdint.h>
# include "velum/cpu.h"

# ifdef VELUM_DEBUG
#  define CPU_FAULT_INJECT 1
# else
#  define CPU_FAULT_INJECT 0
# endif

# define GDT_ENTRIES 7
# define IDT_VECTORS 256
# define EXC_COUNT 32
# define IST_COUNT 3
# define IST_SIZE 16384
# define IST_RESERVE 16
# define BT_MAX 32
# define BT_SPAN 0x10000
# define KERNEL_HALF 0xffff800000000000ull
# define SEG_LIMIT_MAX 0xfffff
# define SEG_KCODE 0x9a
# define SEG_KDATA 0x92
# define SEG_UDATA 0xf2
# define SEG_UCODE 0xfa
# define SEG_TSS 0x89
# define SEG_FLAGS_CODE 0xa
# define SEG_FLAGS_DATA 0xc
# define GATE_INTR 0x8e
# define GATE_DPL3 0x60
# define CR0_MP 0x2ull
# define CR0_EM 0x4ull
# define CR0_TS 0x8ull
# define CR0_NE 0x20ull
# define CR0_WP 0x10000ull
# define CR4_PGE 0x80ull
# define CR4_OSFXSR 0x200ull
# define CR4_OSXMMEXCPT 0x400ull
# define CR4_UMIP 0x800ull
# define CR4_FSGSBASE 0x10000ull
# define CR4_OSXSAVE 0x40000ull
# define CR4_SMEP 0x100000ull
# define CR4_SMAP 0x200000ull
# define EFER_NXE 0x800ull
# define PAT_VALUE 0x0007050100070406ull
# define XCR0_X87 0x1ull
# define XCR0_SSE 0x2ull
# define XCR0_AVX 0x4ull
# define FPU_FCW 0x37f
# define FPU_MXCSR 0x1f80
# define FPU_MXCSR_OFF 24
# define FPU_XSTATE_OFF 512
# define FXSAVE_SIZE 512
# define FPU_ALIGN 64
# define PTE_PRESENT 0x1ull
# define PTE_HUGE 0x80ull
# define PTE_ADDR 0x000ffffffffff000ull
# define FEAT_COUNT 16
# define FAULT_NONE -1
# define CPU_TEST_VEC 0x2f

typedef enum e_cpuidx
{
	CPUID_L0,
	CPUID_L1,
	CPUID_L7,
	CPUID_LD,
	CPUID_E0,
	CPUID_E1,
	CPUID_E2,
	CPUID_E3,
	CPUID_E4,
	CPUID_E7,
	CPUID_E8,
	CPUID_NLEAVES
}	t_cpuidx;

typedef enum e_cpureg
{
	R_EAX,
	R_EBX,
	R_ECX,
	R_EDX
}	t_cpureg;

typedef enum e_trapact
{
	TRAP_HANDLER,
	TRAP_NESTED,
	TRAP_UNHANDLED,
	TRAP_USER_HOOK,
	TRAP_USER_PANIC,
	TRAP_BREAKPOINT,
	TRAP_KERNEL_PANIC
}	t_trapact;

typedef enum e_fault
{
	FAULT_DIV0,
	FAULT_GP,
	FAULT_UD,
	FAULT_PF,
	FAULT_DF,
	FAULT_STACK,
	FAULT_NMI,
	FAULT_COUNT
}	t_fault;

typedef void	(*t_cpuidfn)(uint32_t leaf, uint32_t sub, uint32_t out[4]);

typedef struct s_cpuid_raw
{
	uint32_t	r[CPUID_NLEAVES][4];
}	t_cpuid_raw;

typedef struct s_featbit
{
	const char	*name;
	size_t		offset;
	uint8_t		leaf;
	uint8_t		reg;
	uint8_t		bit;
}	t_featbit;

typedef struct s_trapin
{
	uint64_t	vec;
	bool		user;
	bool		nested;
	bool		handler;
	bool		hook;
}	t_trapin;

typedef struct s_trapslot
{
	t_trapfn	fn;
	void		*ctx;
}	t_trapslot;

typedef struct s_idttab
{
	t_trapslot	slots[IDT_VECTORS];
	bool		lock;
}	t_idttab;

typedef struct s_gdt
{
	uint64_t	e[GDT_ENTRIES];
}	t_gdt;

typedef struct __attribute__((packed)) s_dtr
{
	uint16_t	limit;
	uint64_t	base;
}	t_dtr;

typedef struct __attribute__((packed)) s_tss
{
	uint32_t	reserved0;
	uint64_t	rsp[3];
	uint64_t	reserved1;
	uint64_t	ist[7];
	uint64_t	reserved2;
	uint16_t	reserved3;
	uint16_t	iomap_base;
}	t_tss;

typedef struct s_idt_gate
{
	uint16_t	off_lo;
	uint16_t	sel;
	uint8_t		ist;
	uint8_t		attr;
	uint16_t	off_mid;
	uint32_t	off_hi;
	uint32_t	zero;
}	t_idt_gate;

typedef struct __attribute__((aligned(16))) s_idt
{
	t_idt_gate	gates[IDT_VECTORS];
}	t_idt;

typedef struct s_cputab
{
	t_cpu		cpus[CPU_MAX];
	t_gdt		gdt[CPU_MAX];
	t_tss		tss[CPU_MAX];
	uint64_t	ist_base[CPU_MAX];
}	t_cputab;

typedef struct __attribute__((aligned(16))) s_cpuboot
{
	uint8_t		ist[IST_COUNT * IST_SIZE];
	t_cpuid_raw	raw;
	t_cpufeat	feat;
}	t_cpuboot;

typedef struct s_trapstate
{
	t_userfaultfn	hook;
	uint64_t		breakpoints;
	uint32_t		depth[CPU_MAX];
}	t_trapstate;

typedef struct __attribute__((aligned(64))) s_fpubuf
{
	uint8_t	area[4096];
}	t_fpubuf;

typedef struct s_fpustate
{
	uint64_t	size;
	uint64_t	xcr0;
	bool		xsave;
}	t_fpustate;

uint64_t			gdt_seg_encode(uint32_t base, uint32_t limit,
						uint8_t access, uint8_t flags);
void				gdt_tss_encode(uint64_t out[2], uint64_t base,
						uint32_t limit);
void				gdt_fill(t_gdt *gdt, uint64_t tss_base);
void				tss_fill(t_tss *tss, uint64_t ist1, uint64_t ist2,
						uint64_t ist3);
t_idt_gate			idt_gate_encode(uint64_t fn, uint16_t sel, uint8_t ist,
						uint8_t dpl);
void				cpuid_collect(t_cpuid_raw *raw, t_cpuidfn fn);
void				cpuid_decode(const t_cpuid_raw *raw, t_cpufeat *feat);
void				cpuid_brand(const t_cpuid_raw *raw, char out[49]);
bool				cpuid_bit(const t_cpuid_raw *raw, int leaf, int reg,
						int bit);
const t_featbit		*cpuid_featbits(void);
void				cpufeat_flags(const t_cpufeat *feat, char *buf,
						size_t size);
t_trapact			trap_classify(const t_trapin *in);
uint8_t				idt_vec_ist(uint32_t vec);
uint8_t				idt_vec_dpl(uint32_t vec);
uint64_t			cr0_wanted(uint64_t cr0);
uint64_t			cr4_wanted(uint64_t cr4, const t_cpufeat *feat);
const char			*trap_name(uint64_t vec);
const char			*trap_desc(uint64_t vec);
int					fault_parse(const char *word);
const char			*fault_name(int fault);
int					idt_handler_get(uint8_t vec, t_trapslot *out);
int					bt_walk(uint64_t rbp, uint64_t lo, uint64_t *pcs,
						int max);

void				cpu_tables_init(uint32_t id, uint8_t *ist);
const char			*cpu_stack_name(uint64_t addr);
void				idt_setup(void);
void				cpu_cr_setup(const t_cpufeat *feat);
void				cpu_fpu_boot(const t_cpuid_raw *raw);
const t_cpuid_raw	*cpu_raw(void);
void				trap_dispatch(t_regs *regs);
void				trap_leave_user(bool user);
_Noreturn void		trap_kernel_fault(const t_regs *regs, uint64_t cr2);
void				trap_unhandled(const t_regs *regs);
uint64_t			trap_breakpoints(void);
void				cpu_fault_set(int fault);
int					cpu_fault_run(void);
int					cpu_guard_page(uint64_t va);
int					cpu_selftest_more(void);
int					cpu_st_expect(int cond, const char *what);

void				gdt_load(const t_dtr *gdtr);
void				tss_load(uint16_t sel);
void				idt_load(const t_dtr *idtr);
uint64_t			cpu_read_cr0(void);
void				cpu_write_cr0(uint64_t v);
uint64_t			cpu_read_cr4(void);
void				cpu_write_cr4(uint64_t v);
uint16_t			cpu_read_cs(void);
uint16_t			cpu_read_ss(void);
uint16_t			cpu_read_tr(void);
void				cpu_wbinvd(void);
void				cpu_xsetbv(uint32_t index, uint64_t value);
void				cpu_fpu_hw_reset(void);
void				fpu_xsave(void *area);
void				fpu_xrstor(const void *area);
void				fpu_fxsave(void *area);
void				fpu_fxrstor(const void *area);
void				cpu_trigger_int3(void);
void				cpu_trigger_test_vec(void);
void				cpu_fault_div0(void);
void				cpu_fault_gp(void);
void				cpu_fault_ud(void);
void				cpu_fault_pf(uint64_t addr);
void				cpu_fault_df(void);
void				cpu_fault_stack(void);
void				cpu_fault_nmi(void);

const uint64_t		*isr_stub_table(void);
uint64_t			cpu_boot_stack_low(void);
uint64_t			cpu_fault_rip(int fault);

#endif
