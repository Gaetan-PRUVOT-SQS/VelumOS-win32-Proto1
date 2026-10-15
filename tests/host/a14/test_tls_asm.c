#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include "fake_f4.h"
#include "harness.h"
#include "velum/vtls.h"

static t_vtcb	g_tcb;
static t_vtcb	g_tcb_other;

static uint64_t	tls_fs_base(void)
{
	uint64_t	base;
	uint64_t	args[6];

	base = 0;
	args[0] = F4_ARCH_GET_FS;
	args[1] = f4_u(&base);
	args[2] = 0;
	args[3] = 0;
	args[4] = 0;
	args[5] = 0;
	v_syscall6_real(F4_NR_ARCH_PRCTL, args);
	return (base);
}

static void	tls_layout(void)
{
	h_eq_u64("offsetof(self)", offsetof(t_vtcb, self), V_TCB_OFF_SELF);
	h_eq_u64("offsetof(err)", offsetof(t_vtcb, err), V_TCB_OFF_ERRNO);
	h_eq_u64("offsetof(tid)", offsetof(t_vtcb, tid), V_TCB_OFF_TID);
	h_eq_u64("offsetof(thread)", offsetof(t_vtcb, thread), V_TCB_OFF_THREAD);
	h_eq_u64("sizeof(t_vtcb)", sizeof(t_vtcb), 56);
}

static void	tls_accessors(void)
{
	uint64_t	out[2];

	g_tcb.self = &g_tcb;
	tls_probe(&g_tcb, out);
	h_eq_u64("v_tcb lit fs:0", out[0], f4_u(&g_tcb));
	h_eq_u64("errno = self + 8", out[1], f4_u(&g_tcb.err));
	g_tcb_other.self = &g_tcb;
	tls_probe(&g_tcb_other, out);
	h_eq_u64("v_tcb rend self lu dans fs:0, pas la base de FS",
		out[0], f4_u(&g_tcb));
	g_tcb_other.self = (t_vtcb *)(uintptr_t)0x5000;
	tls_probe(&g_tcb_other, out);
	h_eq_u64("v_tcb rend le contenu de fs:0", out[0], 0x5000);
	h_eq_u64("errno rend self + 8", out[1], 0x5008);
}

static void	tls_restores_fs(void)
{
	uint64_t	before;
	uint64_t	out[2];
	int			i;

	before = tls_fs_base();
	errno = 7;
	i = 0;
	while (i++ < 100)
		tls_probe(&g_tcb, out);
	h_eq_u64("FS d'origine restaure", tls_fs_base(), before);
	h_eq_i64("errno de la libc hote intact", errno, 7);
	h_true(before != 0, "FS de l'hote non nul");
}

int	main(void)
{
	h_begin("a14/tls_asm");
	h_run("t_vtcb/exigence : disposition", tls_layout);
	h_run("v_tcb, errno/exigence : accesseurs asm", tls_accessors);
	h_run("tls_probe/etat : FS restaure", tls_restores_fs);
	return (h_end());
}
