#include <string.h>
#include "harness.h"
#include "blk_int.h"
#include "fake.h"

static t_fkram		g_img;
static t_partlist	g_pl;

static int	parse(const t_fkpart *p, uint32_t n)
{
	t_gptgeo	g;
	t_gpthdr	h;
	int			rc;

	memset(g_img.data, 0, 8192 * 512);
	fk_gpt_build(&g_img, p, n);
	g.ss = 512;
	g.nsectors = 8192;
	g.lba = 1;
	rc = gpt_header(g_img.data + 512, &g, &h);
	if (rc == 0)
		rc = gpt_entries(g_img.data + 1024, &h, &g_pl);
	return (rc);
}

static void	entries_table(void)
{
	static const t_fkpart	ok[2] = {{2048, 4095, 0}, {4096, 6000, 2}};
	static const t_fkpart	ov[2] = {{2048, 4095, 0}, {4095, 5000, 1}};
	static const t_fkpart	lim[2] = {{34, 34, 0}, {8000, 8158, 127}};
	static const t_fkpart	bad[4] = {{33, 40, 0}, {8000, 8159, 0},
	{5000, 4999, 0}, {0, 0, 0}};

	h_eq_i64("valide", parse(ok, 2), 0);
	h_true(g_pl.n == 2 && g_pl.gpt && g_pl.p[1].num == 3, "deux entrees");
	h_eq_i64("chevauchement d'un secteur", parse(ov, 2), E_PROTO);
	h_eq_u64("liste videe", g_pl.n, 0);
	h_eq_i64("limites exactes", parse(lim, 2), 0);
	h_eq_u64("numero de la derniere entree", g_pl.p[1].num, 128);
	h_eq_i64("first_usable - 1", parse(&bad[0], 1), E_PROTO);
	h_eq_i64("last_usable + 1", parse(&bad[1], 1), E_PROTO);
	h_eq_i64("debut > fin", parse(&bad[2], 1), E_PROTO);
	h_eq_i64("lba 0", parse(&bad[3], 1), E_PROTO);
}

static void	entries_unused_and_crc(void)
{
	static const t_fkpart	ok[1] = {{2048, 4095, 0}};
	t_gptgeo				g;
	t_gpthdr				h;

	h_eq_i64("base", parse(ok, 1), 0);
	memset(g_img.data + 1024 + 5 * 128 + 32, 0xff, 16);
	fk_gpt_rehash(&g_img, 1);
	g.ss = 512;
	g.nsectors = 8192;
	g.lba = 1;
	h_eq_i64("en-tete rehache", gpt_header(g_img.data + 512, &g, &h), 0);
	h_eq_i64("type nul ignore malgre ses lba",
		gpt_entries(g_img.data + 1024, &h, &g_pl), 0);
	h_eq_u64("une seule", g_pl.n, 1);
	g_img.data[1024 + 7] ^= 1;
	h_eq_i64("crc du tableau faux", gpt_entries(g_img.data + 1024, &h,
			&g_pl), E_PROTO);
}

static void	entries_many(void)
{
	static t_fkpart	p[33];
	uint32_t		i;

	i = 0;
	while (i < 33)
	{
		p[i].first = 100 + i * 10;
		p[i].last = p[i].first + 5;
		p[i].slot = i;
		i++;
	}
	h_eq_i64("32 partitions", parse(p, 32), 0);
	h_eq_u64("32 gardees", g_pl.n, 32);
	h_eq_i64("33 partitions", parse(p, 33), E_NOTSUP);
}

int	main(void)
{
	fk_ram_init(&g_img, "img", 512, 8192);
	h_begin("a11/gpt_entrees");
	h_run("table des entrees", entries_table);
	h_run("type nul et crc", entries_unused_and_crc);
	h_run("nombre de partitions", entries_many);
	fk_ram_free(&g_img);
	return (h_end());
}
