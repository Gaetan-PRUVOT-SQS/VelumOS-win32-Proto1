#include "harness.h"
#include "th.h"

static const t_deadcase	g_cases[] = {
{"^ e", {0x54, 0x24}, {0xea}},
{"^ a", {0x54, 0x15}, {0xe2}},
{"^ i", {0x54, 0x43}, {0xee}},
{"^ o", {0x54, 0x44}, {0xf4}},
{"^ u", {0x54, 0x3c}, {0xfb}},
{"^ E majuscule", {0x54, 0x24 | TH_SH}, {0xca}},
{"^ espace", {0x54, 0x29}, {'^'}},
{"^ z non composable", {0x54, 0x1d}, {'^', 'z'}},
{"^ ^ meme touche", {0x54, 0x54}, {'^'}},
{"^ puis trema puis espace", {0x54, 0x54 | TH_SH, 0x29}, {'^', 0xa8}},
{"^ chiffre", {0x54, 0x16}, {'^', '&'}},
{"^ Entree", {0x54, 0x5a}, {'^', 0x0d}},
{"trema e", {0x54 | TH_SH, 0x24}, {0xeb}},
{"trema y", {0x54 | TH_SH, 0x35}, {0xff}},
{"trema Y majuscule", {0x54 | TH_SH, 0x35 | TH_SH}, {0x178}},
{"trema espace", {0x54 | TH_SH, 0x29}, {0xa8}},
{"tilde n", {0x1e | TH_AG, 0x31}, {0xf1}},
{"tilde o", {0x1e | TH_AG, 0x44}, {0xf5}},
{"tilde a", {0x1e | TH_AG, 0x15}, {0xe3}},
{"tilde espace", {0x1e | TH_AG, 0x29}, {'~'}},
{"grave e", {0x3d | TH_AG, 0x24}, {0xe8}},
{"grave u", {0x3d | TH_AG, 0x3c}, {0xf9}},
{"grave A majuscule", {0x3d | TH_AG, 0x15 | TH_SH}, {0xc0}},
{"grave espace", {0x3d | TH_AG, 0x29}, {'`'}},
{"sans accent", {0x24}, {'e'}},
{"deux accents de suite", {0x54, 0x24, 0x54, 0x15}, {0xea, 0xe2}},
};

static void	run_table(void)
{
	uint32_t	i;

	i = 0;
	while (i < sizeof(g_cases) / sizeof(g_cases[0]))
	{
		th_run_dead(&g_cases[i]);
		i++;
	}
}

static void	pending_survives_non_chars(void)
{
	t_kbd		k;
	t_inpevent	ev[KEY_OUT_MAX];

	kbd_init(&k);
	h_eq_i64("touche morte : aucun CHAR", th_press(&k, 0x54, ev), 1);
	h_eq_i64("Maj seule : pas de CHAR", th_press(&k, 0x12, ev), 1);
	th_release(&k, 0x12, ev);
	h_eq_i64("F1 : pas de CHAR", th_press(&k, 0x05, ev), 1);
	h_eq_i64("puis e : accent compose", th_press(&k, 0x24, ev), 2);
	h_eq_u64("e circonflexe", ev[1].code, 0xea);
}

static void	release_never_composes(void)
{
	t_kbd		k;
	t_inpevent	ev[KEY_OUT_MAX];

	kbd_init(&k);
	th_press(&k, 0x54, ev);
	h_eq_i64("relachement de la touche morte", th_release(&k, 0x54, ev), 1);
	h_eq_u64("type relachement", ev[0].type, INP_KEY_UP);
	h_eq_i64("accent toujours en attente", th_press(&k, 0x24, ev), 2);
	h_eq_u64("compose", ev[1].code, 0xea);
	h_eq_u64("plus d'accent en attente", k.dead, 0);
}

int	main(void)
{
	h_begin("a10/dead");
	h_run("touches mortes : table", run_table);
	h_run("accent conserve par les non caracteres", pending_survives_non_chars);
	h_run("relachement ne compose pas", release_never_composes);
	return (h_end());
}
