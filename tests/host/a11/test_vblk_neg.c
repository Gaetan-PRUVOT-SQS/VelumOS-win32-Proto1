#include <stdio.h>
#include "harness.h"
#include "vblk.h"
#include "fake.h"

static int	g_idx;

static int	probe_once(t_fkdev *f)
{
	int	base;
	int	rc;

	base = fk_live();
	rc = vblk_probe(&f->pci, (uint32_t)g_idx++);
	if (rc < 0)
		h_eq_i64("memoire rendue apres refus", fk_live(), base);
	h_eq_i64("verrous rendus", g_fk.locks, 0);
	fk_dev_free(f);
	return (rc);
}

static int	probe_case(const t_fkprobe *c)
{
	t_fkdev	*f;

	f = &g_fkdev[0];
	fk_dev_init(f, 1024, VBLK_PCI_DEVICE);
	f->modes = c->modes;
	if (c->features)
		f->features = c->features;
	if (c->qmax != 256)
	{
		f->qmax = c->qmax;
		f->qsize = c->qmax;
	}
	if (c->cfg_off)
		fk_dev_cfg32(f, c->cfg_off, c->cfg_val);
	return (probe_once(f));
}

static void	probe_table(void)
{
	static const t_fkprobe	c[] = {
	{0, 0x200, 256, 0, 0, E_NOTSUP}, {FK_REFUSE_FEAT, 0, 256, 0, 0, E_NOTSUP},
	{0, 0, 0, 0, 0, E_NODEV}, {0, 0, 2, 0, 0, E_NOTSUP},
	{FK_RESET_STUCK, 0, 256, 0, 0, E_IO}, {FK_ENABLE_FAIL, 0, 256, 0, 0, E_IO},
	{FK_MAP_FAIL, 0, 256, 0, 0, E_NOMEM}, {0, 0, 256, 0x04, 0, E_NODEV},
	{0, 0, 256, 0x40, 0x4011, E_NODEV}, {0, 0, 256, 0x58, 0xfffff000, E_NODEV},
	{0, 0, 256, 0x80, 0x02100009, E_NODEV}, {0, 0, 256, 0x90, 3, E_NODEV},
	{FK_GEN_FLAP, 0, 256, 0, 0, E_IO}, {0, 0, 256, 0x5c, 0x37, E_NODEV},
	{0, 0, 256, 0x34, 0xf8, E_NODEV}, {0, 0, 256, 0x54, 6, E_NODEV},
	{0, 0, 1000, 0, 0, 0}, {0, 0, 256, 0x90, 0, 0}, {0, 0, 256, 0, 0, 1}};
	int						i;
	char					what[64];

	i = 0;
	while (c[i].want != 1)
	{
		snprintf(what, sizeof(what), "sonde cas %d", i);
		h_eq_i64(what, probe_case(&c[i]), c[i].want);
		i++;
	}
}

static void	probe_device_values(void)
{
	t_fkdev	*f;

	f = &g_fkdev[0];
	fk_dev_init(f, 1024, VBLK_PCI_DEVICE);
	f->pci.bar_flags[FK_BAR] = PCI_BAR_IO;
	h_eq_i64("BAR d'E/S", probe_once(f), E_NODEV);
	fk_dev_init(f, 1024, VBLK_PCI_DEVICE);
	f->pci.bar_size[FK_BAR] = 0x3fff;
	h_eq_i64("BAR trop petite", probe_once(f), E_NODEV);
	fk_dev_init(f, 1024, VBLK_PCI_DEVICE);
	f->notify_off = 0x400;
	h_eq_i64("notify hors fenetre", probe_once(f), E_IO);
	fk_dev_init(f, 1024, VBLK_PCI_DEVICE);
	f->notify_off = 0x3ff;
	h_eq_i64("notify en fin de fenetre", probe_once(f), 0);
	fk_dev_init(f, 0, VBLK_PCI_DEVICE);
	h_eq_i64("capacite nulle", vblk_probe(&f->pci, 25), E_NODEV);
	h_true(f->status & VIO_ST_FAILED, "FAILED pose apres refus");
	h_eq_i64("27e disque", vblk_probe(&f->pci, 26), E_NOSPC);
	fk_dev_free(f);
}

int	main(void)
{
	h_begin("a11/vblk_sonde");
	h_run("table des refus", probe_table);
	h_run("valeurs de l'appareil", probe_device_values);
	return (h_end());
}
