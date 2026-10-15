#include "harness.h"
#include "th.h"
#include "velum/err.h"

static void	handle_partitions(void)
{
	int64_t	h;

	h = th_sys_setup(0);
	th_push_n(INP_KEY_DOWN, 1, 1);
	h_eq_i64("handle 0", fsys_call(SYS_INPUT_READ, 0, FU_BASE, 1), E_BADF);
	h_eq_i64("handle inexistant", fsys_call(SYS_INPUT_READ, h + 5, FU_BASE,
			1), E_BADF);
	h_eq_i64("handle 2^32 + h", fsys_call(SYS_INPUT_READ, (1ull << 32) + h,
			FU_BASE, 1), E_BADF);
	h_eq_i64("handle -1", fsys_call(SYS_INPUT_READ, ~0ull, FU_BASE, 1),
		E_BADF);
	h_eq_i64("autre type d'objet", fsys_call(SYS_INPUT_READ,
			th_other_handle(), FU_BASE, 1), E_INVAL);
	g_fobj.rights[h - 1] = HR_WAIT;
	h_eq_i64("sans droit HR_READ", fsys_call(SYS_INPUT_READ, h, FU_BASE, 1),
		E_PERM);
	h_eq_i64("references equilibrees", g_fobj.slot[h - 1]->refs, 1);
}

static void	process_and_close(void)
{
	int64_t	h;

	h = th_sys_setup(0);
	th_push_n(INP_KEY_DOWN, 1, 1);
	fproc_clear();
	h_eq_i64("sans processus", fsys_call(SYS_INPUT_READ, h, FU_BASE, 1),
		E_PERM);
	fproc_set(0);
	h_eq_i64("lecture sans PF_INPUT mais handle valide", fsys_call(
			SYS_INPUT_READ, h, FU_BASE, 1), 1);
	handle_close(proc_current(), (t_handle)h);
	h_eq_i64("handle ferme", fsys_call(SYS_INPUT_READ, h, FU_BASE, 1),
		E_BADF);
	h_eq_i64("objet detruit", g_fobj.live, 0);
	h_eq_i64("emplacement rendu", th_open(INPUT_KIND_ALL, NULL) != 0, 1);
}

static void	kind_through_syscall(void)
{
	t_inpevent	*ev;
	int64_t		h;

	h = th_sys_setup(INPUT_KIND_MOUSE);
	th_push_n(INP_KEY_DOWN, 1, 2);
	th_push_n(INP_MOUSE_MOVE, 10, 2);
	th_push_n(INP_CHAR, 20, 1);
	h_eq_i64("souris seule : 2 evenements", fsys_call(SYS_INPUT_READ, h,
			FU_BASE, 10), 2);
	ev = (t_inpevent *)&g_fuser.mem[0];
	h_eq_u64("premier evenement souris", ev[0].code, 10);
	h_eq_u64("type souris", ev[1].type, INP_MOUSE_MOVE);
	h_eq_i64("rien d'autre", fsys_call(SYS_INPUT_READ, h, FU_BASE, 10), 0);
}

static void	two_readers_one_process(void)
{
	int64_t	a;
	int64_t	b;

	a = th_sys_setup(0);
	b = fsys_call(SYS_INPUT_OPEN, 0, 0, 0);
	h_true(a > 0 && b > 0 && a != b, "deux handles distincts");
	th_push_n(INP_KEY_DOWN, 1, 4);
	h_eq_i64("A lit 4", fsys_call(SYS_INPUT_READ, a, FU_BASE, 10), 4);
	h_eq_i64("B lit 4 aussi", fsys_call(SYS_INPUT_READ, b, FU_BASE, 10), 4);
	h_eq_i64("A vide", fsys_call(SYS_INPUT_READ, a, FU_BASE, 10), 0);
}

int	main(void)
{
	h_begin("a10/sys_handles");
	h_run("partitions de handles", handle_partitions);
	h_run("processus et fermeture", process_and_close);
	h_run("genre via appel systeme", kind_through_syscall);
	h_run("deux lecteurs d'un processus", two_readers_one_process);
	return (h_end());
}
