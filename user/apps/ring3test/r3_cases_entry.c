#include "r3.h"

#define R3_AC_BIT 0x40000ull
#define R3_SPIN_CYCLES 150000000ull
#define R3_REGS_OUT 15
#define R3_REGS_PRESERVED 12
#define R3_REGS_PATTERN 0x1111111111111111ull
#define R3_EXC_CODE -1
#define R3_SPIN_CODE 43
#define R3_SYS_OK_CODE 44
#define R3_SYS_KO_CODE 45
#define R3_FLAGS_IF_FIXED 0x202ull

void	r3_regs_syscall(uint64_t out[R3_REGS_OUT]);

__asm__ (
	".text\n"
	".globl r3_regs_syscall\n"
	".type r3_regs_syscall, @function\n"
	"r3_regs_syscall:\n"
	".intel_syntax noprefix\n"
	"push rbx\n"
	"push rbp\n"
	"push r12\n"
	"push r13\n"
	"push r14\n"
	"push r15\n"
	"push rdi\n"
	"mov eax, 7\n"
	"mov rbx, 0x1111111111111111\n"
	"mov rbp, 0x2222222222222222\n"
	"mov rdx, 0x3333333333333333\n"
	"mov rsi, 0x4444444444444444\n"
	"mov rdi, 0x5555555555555555\n"
	"mov r8, 0x6666666666666666\n"
	"mov r9, 0x7777777777777777\n"
	"mov r10, 0x8888888888888888\n"
	"mov r12, 0x9999999999999999\n"
	"mov r13, 0xaaaaaaaaaaaaaaaa\n"
	"mov r14, 0xbbbbbbbbbbbbbbbb\n"
	"mov r15, 0xcccccccccccccccc\n"
	"mov rcx, 0xdddddddddddddddd\n"
	"mov r11, 0xeeeeeeeeeeeeeeee\n"
	"syscall\n"
	"push rax\n"
	"push r11\n"
	"push rcx\n"
	"push r15\n"
	"push r14\n"
	"push r13\n"
	"push r12\n"
	"push r10\n"
	"push r9\n"
	"push r8\n"
	"push rdi\n"
	"push rsi\n"
	"push rdx\n"
	"push rbp\n"
	"push rbx\n"
	"mov rsi, rsp\n"
	"mov rdi, [rsp + 120]\n"
	"mov ecx, 15\n"
	"cld\n"
	"rep movsq\n"
	"add rsp, 128\n"
	"pop r15\n"
	"pop r14\n"
	"pop r13\n"
	"pop r12\n"
	"pop rbp\n"
	"pop rbx\n"
	"ret\n"
	".att_syntax prefix\n"
	".size r3_regs_syscall, . - r3_regs_syscall\n"
);

static void	r3_ac_set(void)
{
	__asm__ volatile ("pushfq\n\torq $0x40000, (%%rsp)\n\tpopfq"
		: : : "memory", "cc");
}

static void	r3_ac_clear(void)
{
	__asm__ volatile ("pushfq\n\tandq $-262145, (%%rsp)\n\tpopfq"
		: : : "memory", "cc");
}

static uint64_t	r3_flags(void)
{
	uint64_t	value;

	__asm__ volatile ("pushfq\n\tpopq %0" : "=r"(value) : : "memory");
	return (value);
}

static uint16_t	r3_ss(void)
{
	uint16_t	value;

	__asm__ volatile ("movw %%ss, %0" : "=r"(value));
	return (value);
}

static uint64_t	r3_tsc(void)
{
	uint32_t	lo;
	uint32_t	hi;

	__asm__ volatile ("rdtsc" : "=a"(lo), "=d"(hi));
	return (((uint64_t)hi << 32) | lo);
}

static int	r3_ac_exception(void)
{
	r3_ac_set();
	__asm__ volatile ("ud2");
	return (99);
}

static int	r3_ac_irq(void)
{
	uint64_t	start;

	r3_ac_set();
	start = r3_tsc();
	while (r3_tsc() - start < R3_SPIN_CYCLES)
		;
	r3_ac_clear();
	return (R3_SPIN_CODE);
}

static int	r3_ac_syscall(void)
{
	int64_t		tid;
	uint64_t	flags;

	r3_ac_set();
	tid = r3_sys(SYS_GETTID, 0, 0, 0);
	flags = r3_flags();
	r3_ac_clear();
	if (tid > 0 && (flags & R3_AC_BIT))
		return (R3_SYS_OK_CODE);
	return (R3_SYS_KO_CODE);
}

int	r3_entry_child(const char *mode)
{
	if (!strcmp(mode, "acexc"))
		return (r3_ac_exception());
	if (!strcmp(mode, "acirq"))
		return (r3_ac_irq());
	if (!strcmp(mode, "acsys"))
		return (r3_ac_syscall());
	return (R3_NO_MODE);
}

static void	r3_entry_expect(t_r3 *t, const char *mode, const char *name,
				int64_t code)
{
	t_procinfo	info;
	int64_t		h;
	int64_t		rc;

	h = r3_child(mode, 0);
	r3_check(t, name, h > 0, 1);
	if (h <= 0)
		return ;
	rc = r3_reap(h, &info);
	r3_check(t, "entrée : enfant attendu", rc, 0);
	r3_check(t, "entrée : code de sortie", (int32_t)info.reserved, code);
}

static void	r3_regs_check(t_r3 *t)
{
	uint64_t	out[R3_REGS_OUT];
	int			bad;
	int			i;

	memset(out, 0, sizeof(out));
	r3_regs_syscall(out);
	r3_check(t, "entrée : SS = 0x1b après sysret", r3_ss(), 0x1b);
	bad = 0;
	i = 0;
	while (i < R3_REGS_PRESERVED)
	{
		if (out[i] != R3_REGS_PATTERN * (uint64_t)(i + 1))
			bad++;
		i++;
	}
	r3_check(t, "entrée : registres préservés par syscall", bad, 0);
	r3_check(t, "entrée : rax reçoit le tid", (int64_t)out[14] > 0, 1);
	r3_check(t, "entrée : rcx reçoit le rip de retour",
		out[12] >= R3_USER_MIN && out[12] < R3_USER_TOP, 1);
	r3_check(t, "entrée : r11 reçoit les drapeaux",
		(out[13] & R3_FLAGS_IF_FIXED) == R3_FLAGS_IF_FIXED, 1);
}

void	r3_suite_entry(t_r3 *t)
{
	r3_regs_check(t);
	r3_entry_expect(t, "acsys", "entrée : AC utilisateur et appel système",
		R3_SYS_OK_CODE);
	r3_entry_expect(t, "acexc", "entrée : AC utilisateur et exception",
		R3_EXC_CODE);
	r3_entry_expect(t, "acirq", "entrée : AC utilisateur et IRQ",
		R3_SPIN_CODE);
}
