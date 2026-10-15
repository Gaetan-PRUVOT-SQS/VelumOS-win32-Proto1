#include <stdint.h>
#include <unistd.h>
#include "fake_f4.h"
#include "harness.h"

static int64_t	stub_call(uint64_t num, uint64_t a0, uint64_t a1, uint64_t a2)
{
	uint64_t	args[6];

	args[0] = a0;
	args[1] = a1;
	args[2] = a2;
	args[3] = 0;
	args[4] = 0;
	args[5] = 0;
	return (v_syscall6_real(num, args));
}

static void	stub_getpid_close(void)
{
	h_eq_i64("getpid reel", stub_call(F4_NR_GETPID, 0, 0, 0), getpid());
	h_eq_i64("close(-1) : -EBADF rendu tel quel",
		stub_call(F4_NR_CLOSE, (uint64_t)-1, 0, 0), -9);
	h_eq_i64("numero inconnu : -ENOSYS rendu tel quel",
		stub_call(100000, 0, 0, 0), -38);
}

static void	stub_pipe_write(void)
{
	int		fds[2];
	char	back[16];
	int64_t	rc;

	h_eq_i64("pipe", pipe(fds), 0);
	rc = stub_call(F4_NR_WRITE, (uint64_t)fds[1], f4_u("bonjour"), 7);
	h_eq_i64("write sur le tube : 7 octets", rc, 7);
	h_eq_i64("lecture du tube", read(fds[0], back, sizeof(back)), 7);
	h_true(back[0] == 'b' && back[6] == 'r', "octets relus");
	h_eq_i64("write sur descripteur invalide",
		stub_call(F4_NR_WRITE, 999, f4_u("x"), 1), -9);
	close(fds[0]);
	close(fds[1]);
}

static void	stub_mmap_six_args(void)
{
	uint64_t	args[6];
	int64_t		addr;
	uint8_t		*p;

	args[0] = 0;
	args[1] = 8192;
	args[2] = 3;
	args[3] = 0x22;
	args[4] = (uint64_t)-1;
	args[5] = 0;
	addr = v_syscall6_real(F4_NR_MMAP, args);
	h_true((uint64_t)addr < (uint64_t)-4095 && (addr & 4095) == 0,
		"mmap a six arguments reussit");
	p = (uint8_t *)(uintptr_t)addr;
	p[0] = 1;
	p[8191] = 2;
	args[0] = (uint64_t)addr;
	h_eq_i64("munmap", v_syscall6_real(F4_NR_MUNMAP, args), 0);
}

int	main(void)
{
	h_begin("a14/stub");
	h_run("v_syscall6/exigence : getpid, erreurs brutes", stub_getpid_close);
	h_run("v_syscall6/exigence : write sur un tube", stub_pipe_write);
	h_run("v_syscall6/exigence : mmap avec r10, r8, r9", stub_mmap_six_args);
	return (h_end());
}
