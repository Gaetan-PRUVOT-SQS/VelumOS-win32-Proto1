#include "harness.h"
#include "smp_int.h"
#include "velum/err.h"

static void	parse_partitions(void)
{
	uint32_t	n;

	h_eq_i64("quatre", smp_parse_count("4", &n), E_OK);
	h_eq_u64("valeur quatre", n, 4);
	h_eq_i64("seize", smp_parse_count("16", &n), E_OK);
	h_eq_u64("valeur seize", n, 16);
	h_eq_i64("zero refuse", smp_parse_count("0", &n), E_INVAL);
	h_eq_i64("vide refuse", smp_parse_count("", &n), E_INVAL);
	h_eq_i64("lettres refusees", smp_parse_count("4x", &n), E_INVAL);
	h_eq_i64("signe refuse", smp_parse_count("-1", &n), E_INVAL);
	h_eq_i64("nul refuse", smp_parse_count(NULL, &n), E_INVAL);
	h_eq_i64("sortie nulle refusee", smp_parse_count("4", NULL), E_INVAL);
}

static void	parse_limits(void)
{
	uint32_t	n;

	h_eq_i64("plafond exact", smp_parse_count("64", &n), E_OK);
	h_eq_u64("valeur plafond", n, CPU_MAX);
	h_eq_i64("au dela du plafond", smp_parse_count("65", &n), E_OK);
	h_eq_u64("valeur bornee", n, CPU_MAX);
	h_eq_i64("enorme", smp_parse_count("99999999999999999999999", &n), E_OK);
	h_eq_u64("enorme borne", n, CPU_MAX);
}

static const uint32_t	g_ids[] = {0, 2, 4, 6, 8, 10};

static t_plan_in	input(uint32_t bsp, uint32_t want)
{
	t_plan_in	in;

	in.ids = g_ids;
	in.count = 6;
	in.bsp = bsp;
	in.want = want;
	in.x2 = false;
	return (in);
}

static void	plan_counts(void)
{
	uint32_t	out[CPU_MAX];
	t_plan_in	in;

	in = input(0, 1);
	h_eq_u64("un coeur demande, aucun ap", smp_plan(&in, out, CPU_MAX), 0);
	in = input(0, 4);
	h_eq_u64("quatre demandes, trois ap", smp_plan(&in, out, CPU_MAX), 3);
	h_eq_u64("premier ap", out[0], 2);
	h_eq_u64("dernier ap", out[2], 6);
	in = input(0, 64);
	h_eq_u64("plus que la MADT", smp_plan(&in, out, CPU_MAX), 5);
	in = input(0, 0);
	h_eq_u64("zero demande", smp_plan(&in, out, CPU_MAX), 0);
	in = input(0, 6);
	h_eq_u64("tous les coeurs de la MADT", smp_plan(&in, out, CPU_MAX), 5);
	h_eq_u64("capacite de sortie", smp_plan(&in, out, 2), 2);
}

static void	plan_bsp_not_first(void)
{
	uint32_t	out[CPU_MAX];
	t_plan_in	in;

	in = input(6, 64);
	h_eq_u64("bsp au milieu", smp_plan(&in, out, CPU_MAX), 5);
	h_eq_u64("bsp absent de la sortie", out[0], 0);
	h_eq_u64("ap suivant", out[2], 4);
	h_eq_u64("dernier ap sans bsp", out[4], 10);
}

static void	plan_duplicates_and_xapic(void)
{
	static const uint32_t	dup[] = {0, 3, 3, 0, 300, 254, 255};
	uint32_t				out[CPU_MAX];
	t_plan_in				in;

	in.ids = dup;
	in.count = 7;
	in.bsp = 0;
	in.want = 64;
	in.x2 = false;
	h_eq_u64("doublons et ids xapic hors limite", smp_plan(&in, out,
			CPU_MAX), 2);
	h_eq_u64("id 3 gardé une fois", out[0], 3);
	h_eq_u64("id 254 gardé", out[1], 254);
	in.x2 = true;
	h_eq_u64("x2apic garde les grands ids", smp_plan(&in, out, CPU_MAX), 4);
	h_eq_u64("grand id 300", out[1], 300);
	h_eq_u64("id 254 apres 300", out[2], 254);
	h_eq_u64("id 255 en dernier", out[3], 255);
}

static void	plan_invalid_input(void)
{
	uint32_t	out[CPU_MAX];
	t_plan_in	in;

	in = input(0, 4);
	h_eq_u64("entree nulle", smp_plan(NULL, out, CPU_MAX), 0);
	h_eq_u64("sortie nulle", smp_plan(&in, NULL, CPU_MAX), 0);
	in.ids = NULL;
	h_eq_u64("liste nulle", smp_plan(&in, out, CPU_MAX), 0);
}

int	main(void)
{
	h_begin("e07/smp_plan");
	h_run("parse_count partitions", parse_partitions);
	h_run("parse_count bornes", parse_limits);
	h_run("plan nombres de coeurs", plan_counts);
	h_run("plan bsp pas en premier", plan_bsp_not_first);
	h_run("plan doublons et xapic", plan_duplicates_and_xapic);
	h_run("plan entrees invalides", plan_invalid_input);
	return (h_end());
}
