#ifndef CPU_H
# define CPU_H

# include <stdbool.h>
# include <stdint.h>

# define GDT_KCODE 0x08
# define GDT_KDATA 0x10
# define GDT_UDATA 0x1b
# define GDT_UCODE 0x23
# define GDT_TSS 0x28
# define IST_DOUBLE_FAULT 1
# define IST_NMI 2
# define IST_MCE 3
# define VEC_PAGE_FAULT 14
# define VEC_IRQ_BASE 0x30
# define VEC_TIMER 0xf0
# define VEC_IPI 0xf1
# define VEC_SPURIOUS 0xff
# define CPU_OFF_SELF 0
# define CPU_OFF_KSTACK 8
# define CPU_OFF_USERRSP 16
# define CPU_OFF_CURRENT 24
# define CPU_MAX 64

typedef struct s_regs
{
	uint64_t	r15;
	uint64_t	r14;
	uint64_t	r13;
	uint64_t	r12;
	uint64_t	r11;
	uint64_t	r10;
	uint64_t	r9;
	uint64_t	r8;
	uint64_t	rbp;
	uint64_t	rdi;
	uint64_t	rsi;
	uint64_t	rdx;
	uint64_t	rcx;
	uint64_t	rbx;
	uint64_t	rax;
	uint64_t	vec;
	uint64_t	err;
	uint64_t	rip;
	uint64_t	cs;
	uint64_t	rflags;
	uint64_t	rsp;
	uint64_t	ss;
}	t_regs;

typedef struct s_cpu
{
	struct s_cpu	*self;
	uint64_t		kstack_top;
	uint64_t		user_rsp;
	struct s_thread	*current;
	uint32_t		id;
	uint32_t		apic_id;
	uint32_t		irq_depth;
	uint32_t		preempt;
	void			*tss;
	void			*sched;
}	t_cpu;

typedef struct s_cpufeat
{
	char		vendor[13];
	char		brand[49];
	uint32_t	family;
	uint32_t	model;
	uint32_t	stepping;
	uint32_t	phys_bits;
	uint32_t	virt_bits;
	bool		nx;
	bool		smep;
	bool		smap;
	bool		umip;
	bool		pcid;
	bool		fsgsbase;
	bool		xsave;
	bool		rdrand;
	bool		rdseed;
	bool		x2apic;
	bool		tsc_deadline;
	bool		invariant_tsc;
	bool		pat;
	bool		pge;
	bool		sse2;
	bool		hypervisor;
}	t_cpufeat;

typedef void	(*t_trapfn)(t_regs *regs, void *ctx);
typedef void	(*t_userfaultfn)(t_regs *regs, uint64_t vec, uint64_t addr);

int				cpu_boot_init(void);
const t_cpufeat	*cpu_features(void);
t_cpu			*cpu_self(void);
uint32_t		cpu_count(void);
void			cpu_halt(void);
void			cpu_cpuid(uint32_t leaf, uint32_t sub, uint32_t out[4]);
int				idt_set_handler(uint8_t vec, t_trapfn fn, void *ctx);
void			idt_clear_handler(uint8_t vec);
void			cpu_set_user_fault(t_userfaultfn fn);
void			gdt_set_rsp0(uint64_t rsp0);
uint64_t		cpu_read_cr2(void);
uint64_t		cpu_read_cr3(void);
void			cpu_write_cr3(uint64_t cr3);
void			cpu_invlpg(uint64_t va);
void			cpu_fpu_init_state(void *area);
void			cpu_fpu_save(void *area);
void			cpu_fpu_restore(const void *area);
uint64_t		cpu_fpu_area_size(void);
int				cpu_backtrace(uint64_t rbp, uint64_t *pcs, int max);
int				cpu_selftest(void);
void			cpu_stack_report(const t_regs *regs, uint64_t cr2);

#endif
