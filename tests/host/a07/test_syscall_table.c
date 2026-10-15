#include <string.h>
#include "a07_fake.h"
#include "harness.h"
#include "proc_sys.h"
#include "velum/err.h"

static void	tab_register(void)
{
	h_eq_i64("numero 0", syscall_register(0, sys_fake_echo, "zero"), 0);
	h_eq_i64("limite 0xfe", syscall_register(SYS_MAX - 1, sys_fake_echo,
			"dernier"), 0);
	h_eq_i64("limite 0xff refuse", syscall_register(SYS_MAX, sys_fake_echo,
			"hors"), E_INVAL);
	h_eq_i64("0x100 refuse", syscall_register(0x100, sys_fake_echo, "x"),
		E_INVAL);
	h_eq_i64("UINT32_MAX refuse", syscall_register(UINT32_MAX,
			sys_fake_echo, "x"), E_INVAL);
	h_eq_i64("fonction nulle", syscall_register(1, NULL, "x"), E_INVAL);
	h_eq_i64("doublon", syscall_register(0, sys_fake_other, "double"),
		E_EXIST);
}

static void	tab_dispatch(void)
{
	t_sysargs	a;
	t_regs		r;

	memset(&a, 0, sizeof(a));
	a.a[0] = 40;
	a.a[5] = 2;
	a.regs = &r;
	h_eq_u64("compteur avant", syscall_calls(0), 0);
	h_eq_i64("premier enregistre garde", syscall_dispatch(&a, 0), 42);
	h_eq_i64("dernier numero", syscall_dispatch(&a, SYS_MAX - 1), 42);
	h_eq_u64("compteur par appel", syscall_calls(0), 1);
	h_eq_u64("compteur hors table", syscall_calls(SYSTAB_SIZE), 0);
	h_eq_i64("enregistrement apres usage",
		syscall_register(7, sys_fake_other, "tard"), 0);
	h_eq_i64("appel tardif", syscall_dispatch(&a, 7), -77);
}

static void	tab_unknown(void)
{
	t_sysargs	a;
	uint64_t	before;
	int			i;

	memset(&a, 0, sizeof(a));
	before = syscall_unknown_count();
	h_eq_i64("numero libre", syscall_dispatch(&a, 5), E_NOSYS);
	h_eq_i64("0xff", syscall_dispatch(&a, SYS_MAX), E_NOSYS);
	h_eq_i64("0x100", syscall_dispatch(&a, SYSTAB_SIZE), E_NOSYS);
	h_eq_i64("UINT32_MAX", syscall_dispatch(&a, UINT32_MAX), E_NOSYS);
	h_eq_u64("inconnus comptes", syscall_unknown_count() - before, 4);
	i = 0;
	while (i < 100)
	{
		syscall_dispatch(&a, 0x1000 + (uint32_t)i);
		i++;
	}
	h_eq_i64("journal borne a 16 lignes", g_fakelog.lines, SYSTAB_LOG_MAX);
	h_eq_u64("compteur continue", syscall_unknown_count() - before, 104);
}

int	main(void)
{
	h_begin("a07/table_appels");
	h_run("enregistrement", tab_register);
	h_run("aiguillage", tab_dispatch);
	h_run("appels inconnus", tab_unknown);
	return (h_end());
}
