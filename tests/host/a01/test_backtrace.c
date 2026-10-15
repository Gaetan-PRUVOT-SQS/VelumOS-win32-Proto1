#include <string.h>
#include "harness.h"
#include "cpu_int.h"

static void	chain(uint64_t *stack, int frames, uint64_t step)
{
	int	i;

	i = 0;
	while (i < frames)
	{
		stack[i * step] = (uint64_t)(stack + (i + 1) * step);
		stack[i * step + 1] = 0x1000 + (uint64_t)i;
		i++;
	}
	stack[frames * step] = 0;
	stack[frames * step + 1] = 0;
}

static void	chaines_valides(void)
{
	static uint64_t	stack[512];
	uint64_t		pcs[BT_MAX];
	int				n;

	chain(stack, 3, 4);
	n = bt_walk((uint64_t)stack, 0, pcs, BT_MAX);
	h_eq_i64("chaine de 3", n, 3);
	h_eq_u64("premier retour", pcs[0], 0x1000);
	h_eq_u64("dernier retour", pcs[2], 0x1002);
	chain(stack, 40, 4);
	h_eq_i64("plafond 32", bt_walk((uint64_t)stack, 0, pcs, BT_MAX), 32);
	h_eq_i64("max 1", bt_walk((uint64_t)stack, 0, pcs, 1), 1);
	h_eq_i64("max 0", bt_walk((uint64_t)stack, 0, pcs, 0), 0);
}

static void	chaines_corrompues(void)
{
	static uint64_t	stack[64];
	uint64_t		pcs[BT_MAX];

	chain(stack, 3, 4);
	stack[4] = (uint64_t)stack;
	h_eq_i64("cadre descendant stoppe", bt_walk((uint64_t)stack, 0, pcs,
			BT_MAX), 2);
	stack[4] = (uint64_t)(stack + 4);
	h_eq_i64("boucle sur soi stoppe", bt_walk((uint64_t)stack, 0, pcs,
			BT_MAX), 2);
	stack[4] = (uint64_t)(stack + 4) + BT_SPAN + 8;
	h_eq_i64("saut > 64 Kio stoppe", bt_walk((uint64_t)stack, 0, pcs,
			BT_MAX), 2);
	h_eq_i64("cadre non aligne", bt_walk((uint64_t)stack + 4, 0, pcs,
			BT_MAX), 0);
	h_eq_i64("sous la borne basse", bt_walk((uint64_t)stack,
			(uint64_t)stack + 8, pcs, BT_MAX), 0);
	h_eq_i64("cadre nul", bt_walk(0, 0, pcs, BT_MAX), 0);
	h_eq_i64("adresse au sommet", bt_walk(UINT64_MAX - 7, 0, pcs, BT_MAX),
		0);
	stack[1] = 0;
	h_eq_i64("retour nul stoppe", bt_walk((uint64_t)stack, 0, pcs, BT_MAX),
		0);
}

static void	api_noyau(void)
{
	static uint64_t	stack[16];
	uint64_t		pcs[BT_MAX];

	chain(stack, 2, 4);
	h_eq_i64("moitie basse refusee", cpu_backtrace((uint64_t)stack, pcs,
			BT_MAX), 0);
	h_eq_i64("pcs nul", cpu_backtrace((uint64_t)stack, NULL, 4), 0);
	h_eq_i64("max negatif", cpu_backtrace((uint64_t)stack, pcs, -1), 0);
	h_eq_i64("max au-dela de 32 borne", cpu_backtrace((uint64_t)stack, pcs,
			1000), 0);
}

int	main(void)
{
	h_begin("a01/backtrace");
	h_run("chaines valides", chaines_valides);
	h_run("chaines corrompues", chaines_corrompues);
	h_run("api noyau", api_noyau);
	return (h_end());
}
