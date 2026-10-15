#include "harness.h"
#include "vblk.h"
#include "fake.h"

static void	alloc_sweep(void)
{
	t_fkdev	*f;
	int		k;
	int		rc;
	int		base;

	f = &g_fkdev[0];
	k = 0;
	rc = -1;
	while (rc < 0 && k < 20)
	{
		fk_dev_init(f, 512, VBLK_PCI_DEVICE);
		base = fk_live();
		fk_fail_after(k);
		rc = vblk_probe(&f->pci, 0);
		fk_fail_after(-1);
		if (rc < 0)
			h_true(rc == E_NOMEM && fk_live() == base, "E_NOMEM, rien garde");
		if (rc < 0)
			fk_dev_free(f);
		k++;
	}
	h_eq_i64("trois points d'echec puis succes", k, 4);
	h_eq_i64("sonde finale", rc, 0);
	h_eq_i64("trois allocations gardees", fk_live(), base + 3);
	h_eq_i64("verrous rendus", g_fk.locks, 0);
}

static void	alloc_scan(void)
{
	static const t_fkpart	p[1] = {{40, 99, 0}};
	t_blkdev				*d;
	int						k;
	int						rc;
	int						base;

	d = blk_find("vda");
	fk_mbr_entry(g_fkdev[0].disk, 0, 0x83, &p[0]);
	base = fk_live();
	k = 0;
	rc = -1;
	while (rc < 0 && k < 10)
	{
		fk_fail_after(k++);
		rc = blk_scan_partitions(d);
		fk_fail_after(-1);
		if (rc < 0)
			h_true(rc == E_NOMEM && fk_live() == base, "scan sans fuite");
	}
	h_eq_i64("scan apres 3 echecs", k, 4);
	h_true(blk_find("vda1") && blk_find("vda1")->first_lba == 40, "vda1");
}

int	main(void)
{
	h_begin("a11/vblk_alloc");
	h_run("echec d'allocation a chaque etape de la sonde", alloc_sweep);
	h_run("echec d'allocation pendant le scan", alloc_scan);
	fk_dev_free(&g_fkdev[0]);
	return (h_end());
}
