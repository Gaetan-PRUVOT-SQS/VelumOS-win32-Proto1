#include <stdio.h>
#include "harness.h"
#include "blk_int.h"
#include "fake.h"

static t_fkram	g_disk;

static void	reg_full(void)
{
	static t_blkdev	d[BLK_MAX_DEVS + 1];
	uint32_t		i;
	int				rc;

	i = 0;
	rc = 0;
	while (i <= BLK_MAX_DEVS && rc == 0)
	{
		d[i] = g_disk.dev;
		snprintf(d[i].name, BLK_NAME_MAX, "x%u", i);
		rc = blk_register(&d[i]);
		i++;
	}
	h_eq_i64("dernier refuse : registre plein", rc, E_NOSPC);
	h_eq_u64("plafond atteint", blk_count(), BLK_MAX_DEVS);
	h_eq_i64("verrous rendus", g_fk.locks, 0);
}

int	main(void)
{
	h_begin("a11/registre_plein");
	fk_ram_init(&g_disk, "vda", 512, 10);
	h_run("registre plein", reg_full);
	fk_ram_free(&g_disk);
	return (h_end());
}
