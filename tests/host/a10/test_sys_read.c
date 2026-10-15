#include "harness.h"
#include "th.h"
#include "../../../kernel/input/inp_sys.h"
#include "velum/err.h"

static void	read_basic(void)
{
	t_inpevent	*ev;
	int64_t		h;

	h = th_sys_setup(0);
	th_push_n(INP_KEY_DOWN, 1, 3);
	h_eq_i64("3 evenements lus", fsys_call(SYS_INPUT_READ, h, FU_BASE, 10), 3);
	ev = (t_inpevent *)&g_fuser.mem[0];
	h_eq_u64("1er code copie", ev[0].code, 1);
	h_eq_u64("3e code copie", ev[2].code, 3);
	h_eq_u64("type copie", ev[1].type, INP_KEY_DOWN);
	h_eq_i64("lecture suivante vide", fsys_call(SYS_INPUT_READ, h, FU_BASE,
			10), 0);
	h_eq_i64("max = 0", fsys_call(SYS_INPUT_READ, h, FU_BASE, 0), 0);
	h_eq_i64("references equilibrees", g_fobj.slot[h - 1]->refs, 1);
	h_eq_i64("jamais de copie sous verrou", g_fuser.lock_violations, 0);
}

static void	read_user_pointer_bounds(void)
{
	int64_t		h;
	uint64_t	end;

	h = th_sys_setup(0);
	th_push_n(INP_KEY_DOWN, 1, 4);
	end = FU_BASE + FU_SIZE;
	h_eq_i64("pointeur noyau", fsys_call(SYS_INPUT_READ, h,
			0xffff800000000000ull, 4), E_FAULT);
	h_eq_i64("pointeur nul", fsys_call(SYS_INPUT_READ, h, 0, 4), E_FAULT);
	h_eq_i64("juste sous la zone", fsys_call(SYS_INPUT_READ, h, FU_BASE - 1,
			1), E_FAULT);
	h_eq_i64("depasse la fin d'un evenement", fsys_call(SYS_INPUT_READ, h,
			end - 32 * 4 + 1, 4), E_FAULT);
	h_eq_i64("aucun evenement perdu par les refus", g_fuser.copies, 0);
	h_eq_i64("exactement a la fin", fsys_call(SYS_INPUT_READ, h,
			end - 32 * 4, 4), 4);
}

static void	read_max_clamp(void)
{
	int64_t	h;

	h = th_sys_setup(0);
	th_push_n(INP_KEY_DOWN, 0, 300);
	h_eq_i64("max enorme borne a 256", fsys_call(SYS_INPUT_READ, h, FU_BASE,
			1ull << 40), 256);
	h_eq_i64("max 2^64-1 : file vide", fsys_call(SYS_INPUT_READ, h, FU_BASE,
			~0ull), 0);
	th_push_n(INP_KEY_DOWN, 0, 17);
	h_eq_i64("17 evenements (2 morceaux)", fsys_call(SYS_INPUT_READ, h,
			FU_BASE, 17), 17);
	th_push_n(INP_KEY_DOWN, 0, 33);
	h_eq_i64("lecture partielle 16", fsys_call(SYS_INPUT_READ, h, FU_BASE,
			16), 16);
	h_eq_i64("puis 17 restants", fsys_call(SYS_INPUT_READ, h, FU_BASE, 100),
		17);
}

static void	read_faults(void)
{
	int64_t	h;

	h = th_sys_setup(0);
	th_push_n(INP_KEY_DOWN, 0, 40);
	g_fuser.fault_addr = FU_BASE + 20 * sizeof(t_inpevent);
	h_eq_i64("faute au 2e morceau : lecture partielle",
		fsys_call(SYS_INPUT_READ, h, FU_BASE, 40), 16);
	h_eq_i64("8 evenements restent", fsys_call(SYS_INPUT_READ, h, FU_BASE + 64,
			100), 8);
	g_fuser.fault_addr = FU_BASE;
	th_push_n(INP_KEY_DOWN, 0, 3);
	h_eq_i64("faute au 1er morceau", fsys_call(SYS_INPUT_READ, h, FU_BASE,
			3), E_FAULT);
	g_fuser.fault_addr = 0;
	h_eq_i64("le morceau fautif est perdu", fsys_call(SYS_INPUT_READ, h,
			FU_BASE, 3), 0);
	h_eq_i64("references equilibrees", g_fobj.slot[h - 1]->refs, 1);
}

int	main(void)
{
	h_begin("a10/sys_read");
	h_run("lecture nominale", read_basic);
	h_run("bornes du pointeur utilisateur", read_user_pointer_bounds);
	h_run("bornage de max et morceaux", read_max_clamp);
	h_run("fautes de copie", read_faults);
	return (h_end());
}
