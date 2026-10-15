#include "harness.h"
#include "th.h"
#include "velum/err.h"

static const t_privcase	g_priv[] = {
{"layout lire, sans privilege", SYS_INPUT_LAYOUT, {0, FU_BASE, 16}, 0, 2},
{"layout lire, avec", SYS_INPUT_LAYOUT, {0, FU_BASE, 16}, PF_INPUT, 2},
{"layout lire court, sans", SYS_INPUT_LAYOUT, {0, FU_BASE, 2}, 0, E_RANGE},
{"layout ecrire, sans", SYS_INPUT_LAYOUT, {1, FU_BASE, 2}, 0, E_PERM},
{"layout ecrire, autres", SYS_INPUT_LAYOUT, {1, FU_BASE, 2}, ~PF_INPUT,
	E_PERM},
{"layout ecrire, avec", SYS_INPUT_LAYOUT, {1, FU_BASE, 2}, PF_INPUT, 0},
{"layout ecrire len 0, sans", SYS_INPUT_LAYOUT, {1, FU_BASE, 0}, 0, E_PERM},
{"layout ecrire len 0, avec", SYS_INPUT_LAYOUT, {1, FU_BASE, 0}, PF_INPUT,
	E_INVAL},
{"layout op 2, sans", SYS_INPUT_LAYOUT, {2, FU_BASE, 2}, 0, E_INVAL},
{"layout op 2, avec", SYS_INPUT_LAYOUT, {2, FU_BASE, 2}, PF_INPUT, E_INVAL},
{"leds 5, sans", SYS_INPUT_LEDS, {5, 0, 0}, 0, E_PERM},
{"leds 5, autres", SYS_INPUT_LEDS, {5, 0, 0}, ~PF_INPUT, E_PERM},
{"leds 5, avec", SYS_INPUT_LEDS, {5, 0, 0}, PF_INPUT, 0},
{"leds 8, sans", SYS_INPUT_LEDS, {8, 0, 0}, 0, E_PERM},
{"leds 8, avec", SYS_INPUT_LEDS, {8, 0, 0}, PF_INPUT, E_INVAL},
{"souris (7,1), sans", SYS_INPUT_MOUSE_CFG, {7, 1, 0}, 0, E_PERM},
{"souris (7,1), autres", SYS_INPUT_MOUSE_CFG, {7, 1, 0}, ~PF_INPUT, E_PERM},
{"souris (7,1), avec", SYS_INPUT_MOUSE_CFG, {7, 1, 0}, PF_INPUT, 0},
{"souris (0,0), sans", SYS_INPUT_MOUSE_CFG, {0, 0, 0}, 0, E_PERM},
{"souris (21,0), avec", SYS_INPUT_MOUSE_CFG, {21, 0, 0}, PF_INPUT, E_INVAL},
{"souris (7,2), avec", SYS_INPUT_MOUSE_CFG, {7, 2, 0}, PF_INPUT, E_INVAL},
};

static void	decision_table(void)
{
	const t_privcase	*c;
	uint32_t			i;

	i = 0;
	while (i < sizeof(g_priv) / sizeof(g_priv[0]))
	{
		c = &g_priv[i];
		th_sys_setup(0);
		memcpy(&g_fuser.mem[0], "us", 3);
		fproc_set(c->flags);
		h_eq_i64(c->name, fsys_call(c->num, c->a[0], c->a[1], c->a[2]),
			c->exp);
		i++;
	}
}

static void	refusal_keeps_state(void)
{
	th_sys_setup(0);
	memcpy(&g_fuser.mem[0], "fr\0us", 6);
	h_eq_i64("fr avec privilege", fsys_call(SYS_INPUT_LAYOUT, 1, FU_BASE, 2),
		0);
	h_eq_i64("leds 2", fsys_call(SYS_INPUT_LEDS, 2, 0, 0), 0);
	h_eq_i64("souris (9,1)", fsys_call(SYS_INPUT_MOUSE_CFG, 9, 1, 0), 0);
	fproc_set(PF_DISPLAY | PF_SPAWN);
	h_eq_i64("us refuse", fsys_call(SYS_INPUT_LAYOUT, 1, FU_BASE + 3, 2),
		E_PERM);
	h_eq_i64("leds refusees", fsys_call(SYS_INPUT_LEDS, 5, 0, 0), E_PERM);
	h_eq_i64("souris refusee", fsys_call(SYS_INPUT_MOUSE_CFG, 3, 0, 0), E_PERM);
	h_eq_str("disposition inchangee", input_layout(), "fr");
	h_eq_u64("verrous inchanges", input_leds(), 2);
	h_eq_u64("vitesse inchangee", g_xlate.mouse.accel.speed, 9);
	h_eq_u64("acceleration inchangee", g_xlate.mouse.accel.enabled, 1);
}

static void	no_process(void)
{
	th_sys_setup(0);
	memcpy(&g_fuser.mem[0], "us", 3);
	fproc_clear();
	h_eq_i64("layout ecrire sans processus", fsys_call(SYS_INPUT_LAYOUT, 1,
			FU_BASE, 2), E_PERM);
	h_eq_i64("leds sans processus", fsys_call(SYS_INPUT_LEDS, 1, 0, 0), E_PERM);
	h_eq_i64("souris sans processus", fsys_call(SYS_INPUT_MOUSE_CFG, 5, 0, 0),
		E_PERM);
}

int	main(void)
{
	h_begin("a10/sys_priv");
	h_run("table de decision appel x operation x privilege x argument",
		decision_table);
	h_run("refus sans effet sur l'etat global", refusal_keeps_state);
	h_run("appel sans processus courant", no_process);
	return (h_end());
}
