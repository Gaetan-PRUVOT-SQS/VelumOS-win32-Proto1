#include <stdint.h>
#include <unistd.h>
#include "fake_f4.h"
#include "harness.h"

static void	regs_preserved(void)
{
	h_eq_i64("rbx, rbp, r12-r15 preserves", stub_preserve_check(), 0);
}

static void	regs_argument_registers(void)
{
	uint64_t	args[6];

	args[0] = 0;
	args[1] = 4096;
	args[2] = 3;
	args[3] = 0x2;
	args[4] = (uint64_t)-1;
	args[5] = 0;
	h_eq_i64("r10 lu : fd -1 sans MAP_ANONYMOUS -> EBADF",
		v_syscall6_real(F4_NR_MMAP, args), -9);
	args[3] = 0x20;
	h_eq_i64("r10 lu : ni PRIVATE ni SHARED -> EINVAL",
		v_syscall6_real(F4_NR_MMAP, args), -22);
	args[3] = 0x22;
	args[5] = 1;
	h_eq_i64("r9 lu : offset non aligne -> EINVAL",
		v_syscall6_real(F4_NR_MMAP, args), -22);
}

static void	regs_repeated(void)
{
	uint64_t	args[6];
	int			i;
	int			bad;

	args[0] = 0;
	args[1] = 0;
	args[2] = 0;
	args[3] = 0;
	args[4] = 0;
	args[5] = 0;
	bad = 0;
	i = 0;
	while (i++ < 1000)
		bad += v_syscall6_real(F4_NR_GETPID, args) != getpid();
	h_eq_i64("1000 appels identiques", bad, 0);
	h_eq_u64("tableau d'arguments inchange", args[0] | args[1] | args[5], 0);
}

int	main(void)
{
	h_begin("a14/stub_regs");
	h_run("v_syscall6/exigence : registres preserves", regs_preserved);
	h_run("v_syscall6/partition : r10 et r9", regs_argument_registers);
	h_run("v_syscall6/aleatoire : 1000 appels", regs_repeated);
	return (h_end());
}
