#include <stdio.h>
#include <string.h>
#include "harness.h"
#include "blk_int.h"
#include "fake.h"

static t_fkram	g_disk;

static void	reg_valid_and_find(void)
{
	fk_ram_init(&g_disk, "vda", 512, 1000);
	h_eq_i64("enregistrement", blk_register(&g_disk.dev), 0);
	h_true(blk_find("vda") == &g_disk.dev, "trouve vda");
	h_true(blk_find("vdb") == NULL, "vdb absent");
	h_true(blk_find("vd") == NULL, "prefixe refuse");
	h_true(blk_find("vda1234567890abcdef") == NULL, "nom trop long");
	h_true(blk_find(NULL) == NULL, "nom nul");
	h_eq_u64("compte", blk_count(), 1);
	h_true(blk_at(0) == &g_disk.dev, "indice 0");
	h_true(blk_at(1) == NULL, "indice hors table");
	h_true(blk_at(UINT32_MAX) == NULL, "indice maximal");
	h_eq_i64("verrous rendus", g_fk.locks, 0);
}

static int	try_case(const t_fkcase *c)
{
	static const t_blkops	none = {NULL, NULL, NULL};
	static t_blkdev			d;
	t_blkops				part;

	part = *g_disk.dev.ops;
	part.flush = NULL;
	if (c->ops == 3)
		part.write = NULL;
	memset(&d, 0, sizeof(d));
	memcpy(d.name, c->name, strnlen(c->name, BLK_NAME_MAX));
	d.sector_size = c->ss;
	d.nsectors = c->ns;
	d.first_lba = c->first;
	d.ops = g_disk.dev.ops;
	if (c->ops == 0)
		d.ops = NULL;
	if (c->ops == 2)
		d.ops = &none;
	if (c->ops >= 3)
		d.ops = &part;
	return (blk_register(&d));
}

static void	reg_decision_table(void)
{
	static const t_fkcase	cases[] = {
	{"", 512, 10, 0, 1, E_INVAL}, {"aaaaaaaaaaaaaaaa", 512, 10, 0, 1, E_INVAL},
	{"Vdx", 512, 10, 0, 1, E_INVAL}, {"v/x", 512, 10, 0, 1, E_INVAL},
	{"vdx", 256, 10, 0, 1, E_INVAL}, {"vdx", 8192, 10, 0, 1, E_INVAL},
	{"vdx", 768, 10, 0, 1, E_INVAL}, {"vdx", 512, 0, 0, 1, E_INVAL},
	{"vdx", 512, 10, 0, 0, E_INVAL}, {"vdx", 512, 10, 0, 2, E_INVAL},
	{"vdx", 512, 10, 3, 1, E_INVAL}, {"vda", 512, 10, 0, 1, E_EXIST},
	{"v{x", 512, 10, 0, 1, E_INVAL}, {"v:x", 512, 10, 0, 1, E_INVAL},
	{"vdx", 512, 10, 0, 3, E_INVAL}, {"vdx", 512, 10, 0, 4, E_INVAL},
	{NULL, 0, 0, 0, 0, 0}};
	int						i;
	char					what[64];

	h_eq_i64("pointeur nul", blk_register(NULL), E_INVAL);
	i = 0;
	while (cases[i].name)
	{
		snprintf(what, sizeof(what), "cas %d", i);
		h_eq_i64(what, try_case(&cases[i]), cases[i].want);
		i++;
	}
	h_eq_u64("rien d'enregistre", blk_count(), 1);
}

static void	reg_partitions(void)
{
	static t_blkdev	p[8];
	int				i;

	memcpy(p[0].name, "vda1", 5);
	p[0].sector_size = 512;
	p[0].nsectors = 100;
	p[0].first_lba = 10;
	p[0].parent = &g_disk.dev;
	i = 1;
	while (i < 8)
		p[i++] = p[0];
	p[7].parent = NULL;
	p[1].parent = &p[7];
	p[1].first_lba = 0;
	p[2].parent = &p[0];
	p[3].sector_size = 4096;
	p[4].first_lba = 950;
	p[6].first_lba = 1000;
	h_eq_i64("parent inconnu", blk_register(&p[1]), E_INVAL);
	h_eq_i64("partition de partition", blk_register(&p[2]), E_INVAL);
	h_eq_i64("taille de secteur differente", blk_register(&p[3]), E_INVAL);
	h_eq_i64("deborde du parent", blk_register(&p[4]), E_RANGE);
	h_eq_i64("debut = fin du parent", blk_register(&p[6]), E_RANGE);
	h_eq_i64("partition valide", blk_register(&p[0]), 0);
	h_eq_i64("meme pointeur", blk_register(&p[0]), E_EXIST);
}

int	main(void)
{
	h_begin("a11/registre");
	h_run("enregistrement et recherche", reg_valid_and_find);
	h_run("table de decision", reg_decision_table);
	h_run("partitions", reg_partitions);
	fk_ram_free(&g_disk);
	return (h_end());
}
