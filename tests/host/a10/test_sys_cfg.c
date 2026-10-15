#include "harness.h"
#include "th.h"
#include "velum/err.h"

static void	layout_get(void)
{
	char	*mem;

	th_sys_setup(0);
	mem = (char *)&g_fuser.mem[0];
	h_eq_i64("lecture : longueur", fsys_call(SYS_INPUT_LAYOUT, 0, FU_BASE, 16),
		2);
	h_eq_str("nom ecrit", mem, "fr");
	h_eq_i64("tampon trop petit (2)", fsys_call(SYS_INPUT_LAYOUT, 0, FU_BASE,
			2), E_RANGE);
	h_eq_i64("tampon exact (3)", fsys_call(SYS_INPUT_LAYOUT, 0, FU_BASE, 3),
		2);
	h_eq_i64("tampon nul", fsys_call(SYS_INPUT_LAYOUT, 0, FU_BASE, 0),
		E_RANGE);
	h_eq_i64("pointeur noyau", fsys_call(SYS_INPUT_LAYOUT, 0,
			0xffff800000000000ull, 16), E_FAULT);
	h_eq_i64("operation inconnue", fsys_call(SYS_INPUT_LAYOUT, 2, FU_BASE, 16),
		E_INVAL);
	h_eq_i64("operation 2^32", fsys_call(SYS_INPUT_LAYOUT, 1ull << 32,
			FU_BASE, 16), E_INVAL);
}

static void	layout_set(void)
{
	th_sys_setup(0);
	memcpy(&g_fuser.mem[0], "us\0xx", 6);
	h_eq_i64("us", fsys_call(SYS_INPUT_LAYOUT, 1, FU_BASE, 2), 0);
	h_eq_str("disposition active", input_layout(), "us");
	h_eq_i64("inconnue", fsys_call(SYS_INPUT_LAYOUT, 1, FU_BASE + 3, 2),
		E_NOENT);
	h_eq_i64("longueur 0", fsys_call(SYS_INPUT_LAYOUT, 1, FU_BASE, 0), E_INVAL);
	h_eq_i64("longueur 16", fsys_call(SYS_INPUT_LAYOUT, 1, FU_BASE, 16),
		E_INVAL);
	h_eq_i64("longueur 2^40", fsys_call(SYS_INPUT_LAYOUT, 1, FU_BASE,
			1ull << 40), E_INVAL);
	h_eq_i64("octet nul integre", fsys_call(SYS_INPUT_LAYOUT, 1, FU_BASE, 3),
		E_INVAL);
	h_eq_i64("pointeur hors zone", fsys_call(SYS_INPUT_LAYOUT, 1, 8, 2),
		E_FAULT);
	h_eq_str("inchangee apres refus", input_layout(), "us");
}

static void	leds(void)
{
	uint64_t	m;

	th_sys_setup(0);
	m = 0;
	while (m <= 7)
	{
		h_eq_i64("masque valide", fsys_call(SYS_INPUT_LEDS, m, 0, 0), 0);
		h_eq_u64("etat des verrous", input_leds(), m);
		m++;
	}
	h_eq_i64("masque 8", fsys_call(SYS_INPUT_LEDS, 8, 0, 0), E_INVAL);
	h_eq_i64("masque 2^32 + 1", fsys_call(SYS_INPUT_LEDS, (1ull << 32) + 1, 0,
			0), E_INVAL);
	h_eq_i64("masque -1", fsys_call(SYS_INPUT_LEDS, ~0ull, 0, 0), E_INVAL);
	h_eq_u64("verrous inchanges apres refus", input_leds(), 7);
}

static void	mouse_cfg(void)
{
	th_sys_setup(0);
	h_eq_i64("(1,0)", fsys_call(SYS_INPUT_MOUSE_CFG, 1, 0, 0), 0);
	h_eq_i64("(20,1)", fsys_call(SYS_INPUT_MOUSE_CFG, 20, 1, 0), 0);
	h_eq_u64("vitesse", g_xlate.mouse.accel.speed, 20);
	h_eq_i64("vitesse 0", fsys_call(SYS_INPUT_MOUSE_CFG, 0, 0, 0), E_INVAL);
	h_eq_i64("vitesse 21", fsys_call(SYS_INPUT_MOUSE_CFG, 21, 0, 0), E_INVAL);
	h_eq_i64("acceleration 2", fsys_call(SYS_INPUT_MOUSE_CFG, 10, 2, 0),
		E_INVAL);
	h_eq_i64("vitesse 2^32 + 5", fsys_call(SYS_INPUT_MOUSE_CFG,
			(1ull << 32) + 5, 0, 0), E_INVAL);
	h_eq_i64("acceleration 2^32 + 1", fsys_call(SYS_INPUT_MOUSE_CFG, 5,
			(1ull << 32) + 1, 0), E_INVAL);
	h_eq_u64("etat inchange apres refus", g_xlate.mouse.accel.speed, 20);
	h_eq_u64("acceleration inchangee", g_xlate.mouse.accel.enabled, 1);
}

int	main(void)
{
	h_begin("a10/sys_cfg");
	h_run("SYS_INPUT_LAYOUT lecture", layout_get);
	h_run("SYS_INPUT_LAYOUT ecriture", layout_set);
	h_run("SYS_INPUT_LEDS", leds);
	h_run("SYS_INPUT_MOUSE_CFG", mouse_cfg);
	return (h_end());
}
