#include "harness.h"
#include "blk_int.h"

static void	names_simple(void)
{
	char	out[BLK_NAME_MAX];

	h_eq_i64("vda 1", blk_part_name(out, "vda", 1), 0);
	h_eq_str("vda1", out, "vda1");
	h_eq_i64("vdz 128", blk_part_name(out, "vdz", 128), 0);
	h_eq_str("vdz128", out, "vdz128");
	h_eq_i64("sda 9999", blk_part_name(out, "sda", 9999), 0);
	h_eq_str("sda9999", out, "sda9999");
	h_eq_i64("nvme0n1 1", blk_part_name(out, "nvme0n1", 1), 0);
	h_eq_str("p intercale", out, "nvme0n1p1");
	h_eq_i64("mmc0 12", blk_part_name(out, "mmc0", 12), 0);
	h_eq_str("mmc0p12", out, "mmc0p12");
}

static void	names_limits(void)
{
	char	out[BLK_NAME_MAX];

	h_eq_i64("numero 0", blk_part_name(out, "vda", 0), E_INVAL);
	h_eq_i64("numero 10000", blk_part_name(out, "vda", 10000), E_INVAL);
	h_eq_i64("15 octets tient", blk_part_name(out, "abcdefghijklm", 99), 0);
	h_eq_str("limite exacte", out, "abcdefghijklm99");
	h_eq_i64("16 octets refuse", blk_part_name(out, "abcdefghijklm", 100),
		E_RANGE);
	h_eq_i64("p qui deborde", blk_part_name(out, "abcdefghijkl1", 10),
		E_RANGE);
	h_eq_i64("disque vide", blk_part_name(out, "", 1), E_RANGE);
}

static t_partlist	g_pl;

static void	parts_add(void)
{
	t_part	p;

	p.first = 100;
	p.count = 10;
	p.num = 1;
	h_eq_i64("ajout", part_add(&g_pl, &p), 0);
	p.first = 109;
	h_eq_i64("chevauche le dernier secteur", part_add(&g_pl, &p), E_PROTO);
	p.first = 90;
	p.count = 11;
	h_eq_i64("chevauche le premier", part_add(&g_pl, &p), E_PROTO);
	p.count = 10;
	h_eq_i64("adjacent avant accepte", part_add(&g_pl, &p), 0);
	p.count = 0;
	h_eq_i64("taille nulle", part_add(&g_pl, &p), E_PROTO);
	p.first = UINT64_MAX;
	p.count = 2;
	h_eq_i64("enroulement 64 bits", part_add(&g_pl, &p), E_PROTO);
}

static void	parts_fill(void)
{
	t_part		p;
	uint32_t	i;

	i = 0;
	while (i++ < BLK_PARTS_MAX)
	{
		p.first = 1000 + i * 10;
		p.count = 1;
		p.num = i;
		part_add(&g_pl, &p);
	}
	h_eq_u64("plafond", g_pl.n, BLK_PARTS_MAX);
	p.first = 5000;
	h_eq_i64("33e partition", part_add(&g_pl, &p), E_NOTSUP);
}

int	main(void)
{
	h_begin("a11/partname");
	h_run("noms simples", names_simple);
	h_run("limites", names_limits);
	h_run("liste de partitions", parts_add);
	h_run("plafond de partitions", parts_fill);
	return (h_end());
}
