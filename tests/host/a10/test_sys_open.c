#include "harness.h"
#include "th.h"
#include "../../../kernel/input/inp_sys.h"
#include "velum/err.h"

static void	registration(void)
{
	uint32_t	n;

	fake_all_reset();
	h_eq_i64("5 appels enregistres", inp_sys_register(), 0);
	n = SYS_INPUT_OPEN;
	while (n <= SYS_INPUT_MOUSE_CFG)
	{
		h_true(g_fsys.fn[n] != NULL, "appel 0x7x present");
		n++;
	}
	h_eq_i64("double enregistrement refuse", inp_sys_register(), 5);
	h_eq_i64("appel voisin 0x75 absent", fsys_call(0x75, 0, 0, 0), E_NOSYS);
}

static void	privileges(void)
{
	fake_all_reset();
	inp_sys_register();
	fproc_clear();
	h_eq_i64("sans processus", fsys_call(SYS_INPUT_OPEN, 0, 0, 0), E_PERM);
	fproc_set(PF_ALL ^ PF_INPUT);
	h_eq_i64("sans PF_INPUT", fsys_call(SYS_INPUT_OPEN, 0, 0, 0), E_PERM);
	h_eq_i64("sans PF_INPUT : rien d'ouvert", g_fobj.live, 0);
	fproc_set(PF_INPUT);
	h_true(fsys_call(SYS_INPUT_OPEN, 0, 0, 0) > 0, "avec PF_INPUT");
	h_eq_i64("un objet", g_fobj.live, 1);
}

static void	kind_partitions(void)
{
	fake_all_reset();
	inp_sys_register();
	fproc_set(PF_INPUT);
	h_true(fsys_call(SYS_INPUT_OPEN, 0, 0, 0) > 0, "genre 0");
	h_true(fsys_call(SYS_INPUT_OPEN, 1, 0, 0) > 0, "genre 1");
	h_true(fsys_call(SYS_INPUT_OPEN, 2, 0, 0) > 0, "genre 2");
	h_eq_i64("genre 3", fsys_call(SYS_INPUT_OPEN, 3, 0, 0), E_INVAL);
	h_eq_i64("genre 2^32", fsys_call(SYS_INPUT_OPEN, 1ull << 32, 0, 0),
		E_INVAL);
	h_eq_i64("genre -1", fsys_call(SYS_INPUT_OPEN, ~0ull, 0, 0), E_INVAL);
	h_eq_i64("trois objets", g_fobj.live, 3);
}

static void	resource_failures(void)
{
	int64_t	h;

	fake_all_reset();
	inp_sys_register();
	fproc_set(PF_INPUT);
	g_fobj.fail_create = 1;
	h_eq_i64("memoire objet", fsys_call(SYS_INPUT_OPEN, 0, 0, 0), E_NOMEM);
	g_fobj.fail_create = 0;
	g_fobj.fail_handle = 1;
	h_eq_i64("table de handles", fsys_call(SYS_INPUT_OPEN, 0, 0, 0), E_NOMEM);
	g_fobj.fail_handle = 0;
	h = fsys_call(SYS_INPUT_OPEN, 0, 0, 0);
	h_true(h > 0, "reessai apres echecs");
	h_eq_i64("pas de fuite d'objet", g_fobj.live, 1);
}

int	main(void)
{
	h_begin("a10/sys_open");
	h_run("enregistrement des appels", registration);
	h_run("privilege PF_INPUT", privileges);
	h_run("partitions du genre", kind_partitions);
	h_run("echecs de ressources", resource_failures);
	return (h_end());
}
