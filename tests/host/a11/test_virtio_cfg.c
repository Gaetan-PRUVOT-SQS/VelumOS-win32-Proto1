#include "harness.h"
#include "vblk.h"
#include "fake.h"

static void	cfg_reads(void)
{
	t_vio		v;
	uint64_t	val;

	fk_dev_init(&g_fkdev[0], 0x1234, VBLK_PCI_DEVICE);
	h_eq_i64("capacites", vio_find_caps(&g_fkdev[0].pci, &v), 0);
	h_eq_i64("projection", vio_map(&v), 0);
	val = 0;
	h_eq_i64("octet", vio_cfg_read(&v, 0, &val, 1), 0);
	h_eq_u64("octet lu", val, 0x34);
	h_eq_i64("mot", vio_cfg_read(&v, 0, &val, 2), 0);
	h_eq_u64("mot lu", val, 0x1234);
	h_eq_i64("largeur 3", vio_cfg_read(&v, 0, &val, 3), E_INVAL);
	h_eq_i64("mal aligne", vio_cfg_read(&v, 2, &val, 4), E_RANGE);
	h_eq_i64("fin de fenetre", vio_cfg_read(&v, 0xffc, &val, 4), 0);
	h_eq_i64("hors fenetre", vio_cfg_read(&v, 0x1000, &val, 1), E_RANGE);
	h_eq_i64("deborde 32 bits", vio_cfg_read(&v, 0xfffffff8, &val, 8),
		E_RANGE);
	v.devcfg = NULL;
	h_eq_i64("sans config", vio_cfg_read(&v, 0, &val, 1), E_NODEV);
	fk_dev_free(&g_fkdev[0]);
}

int	main(void)
{
	h_begin("a11/virtio_cfg");
	h_run("lecture de configuration", cfg_reads);
	return (h_end());
}
