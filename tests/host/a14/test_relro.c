#include <stdint.h>
#include "fake_check.h"
#include "fake_f4.h"
#include "fake_sys.h"
#include "harness.h"
#include "velum/abi/abi_syscall.h"
#include "velum/err.h"

static int	relro_call(uint64_t start, uint64_t end, int64_t kernel_rc)
{
	fake_reset();
	fake_kernel_on();
	g_fsys.vprotect_ret = kernel_rc;
	return (relro_lock(start, end));
}

static void	relro_ranges(void)
{
	h_eq_i64("plage alignee", relro_call(0x10000, 0x12000, 0), 0);
	sys_is(SYS_VPROTECT, 0x10000, 0x2000, PROT_R);
	h_eq_u64("une seule requete", g_fsys.calls, 1);
	h_eq_i64("plage desalignee", relro_call(0x10001, 0x13fff, 0), 0);
	sys_is(SYS_VPROTECT, 0x11000, 0x2000, PROT_R);
	h_eq_i64("une page", relro_call(0x20000, 0x21000, 0), 0);
	sys_is(SYS_VPROTECT, 0x20000, 0x1000, PROT_R);
	h_eq_i64("erreur du noyau propagee", relro_call(0x20000, 0x23000,
			E_PERM), E_PERM);
	sys_is(SYS_VPROTECT, 0x20000, 0x3000, PROT_R);
	h_eq_u64("requete faite malgre l'erreur", g_fsys.calls, 1);
}

static void	relro_degenerate(void)
{
	h_eq_i64("plage vide alignee", relro_call(0x30000, 0x30000, 0), 0);
	sys_none();
	h_eq_i64("debut apres la fin", relro_call(0x20000, 0x10000, 0), E_INVAL);
	sys_none();
	h_eq_i64("sans page entiere", relro_call(0x10001, 0x10fff, 0), 0);
	sys_none();
	h_eq_i64("vide mais desalignee", relro_call(0x10010, 0x10010, 0), 0);
	sys_none();
}

static void	relro_overflow(void)
{
	h_eq_i64("debut a 4094 du sommet", relro_call(UINT64_MAX - 4094,
			UINT64_MAX, 0), E_INVAL);
	sys_none();
	h_eq_i64("debut a UINT64_MAX", relro_call(UINT64_MAX, UINT64_MAX, 0),
		E_INVAL);
	sys_none();
	h_eq_i64("derniere page alignee et vide", relro_call(UINT64_MAX - 4095,
			UINT64_MAX, 0), 0);
	sys_none();
	h_eq_i64("derniere page pleine", relro_call(0xffffffffffffe000ull,
			UINT64_MAX, 0), 0);
	sys_is(SYS_VPROTECT, 0xffffffffffffe000ull, 0x1000, PROT_R);
}

int	main(void)
{
	h_begin("a14/relro");
	h_run("relro_lock/partition : plages valides", relro_ranges);
	h_run("relro_lock/partition : plages degenerees", relro_degenerate);
	h_run("relro_lock/valeur limite : sommet de l'espace", relro_overflow);
	return (h_end());
}
