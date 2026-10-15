#include "cases.h"
#include "fakes.h"
#include "velum/err.h"

static void	detect_table(void)
{
	t_dispi	d;
	int		i;

	i = 0;
	while (g_id_cases[i].name)
	{
		g_h.name = g_id_cases[i].name;
		fake_dispi_reset();
		g_fd.id_reply = g_id_cases[i].id;
		g_fd.vram64k = g_id_cases[i].vram64k;
		g_fd.bar_bytes = g_id_cases[i].bar;
		fake_dispi_bind(&d);
		h_eq_i64("dispi_detect", dispi_detect(&d), g_id_cases[i].want);
		h_true(d.ready == (g_id_cases[i].want == E_OK), "etat pret");
		h_eq_u64("vram retenue", d.vram, g_id_cases[i].vram);
		h_true(g_fd.log[0][0] == 0 && g_fd.log[0][1] == DISPI_ID_WRITE,
			"identifiant d'abord ecrit puis relu");
		i++;
	}
}

static void	detect_no_ops(void)
{
	t_dispi	d;

	memset(&d, 0, sizeof(d));
	h_eq_i64("aucune operation", dispi_detect(&d), E_NODEV);
	d.ops.read = fake_dispi_read;
	h_eq_i64("ecriture absente", dispi_detect(&d), E_NODEV);
	h_true(!d.ready, "jamais pret");
}

static void	pci_match(void)
{
	t_pcidev	dev;

	memset(&dev, 0, sizeof(dev));
	dev.bar[0] = FAKE_FB_PHYS;
	dev.bar_size[0] = 16777216;
	h_true(dispi_pci_match(&dev, FAKE_FB_PHYS), "BAR0 egale au framebuffer");
	h_true(!dispi_pci_match(NULL, FAKE_FB_PHYS), "appareil absent");
	h_true(!dispi_pci_match(&dev, FAKE_FB_PHYS + 4096), "framebuffer decale");
	h_true(!dispi_pci_match(&dev, 0), "framebuffer nul");
	dev.bar_size[0] = 0;
	h_true(!dispi_pci_match(&dev, FAKE_FB_PHYS), "BAR0 de taille nulle");
	dev.bar_size[0] = 16777216;
	dev.bar_flags[0] = PCI_BAR_IO;
	h_true(!dispi_pci_match(&dev, FAKE_FB_PHYS), "BAR0 en espace E/S");
	dev.bar_flags[0] = PCI_BAR_MEM64;
	dev.bar[0] = 0x1000000000000ull;
	h_true(dispi_pci_match(&dev, 0x1000000000000ull), "BAR 64 bits haute");
}

static void	mmio_layout(void)
{
	t_dispi		d;
	uint16_t	regs[2048];
	int			i;

	memset(&d, 0, sizeof(d));
	fake_regs_fill(regs, 2048, 0xa5a5);
	dispi_mmio_bind(&d, regs);
	i = 0;
	while (i <= DISPI_INDEX_VRAM64K)
	{
		dispi_wr(&d, (uint16_t)i, (uint16_t)(0x1001 + i));
		i++;
	}
	h_eq_u64("index 1 a l'octet 0x502", regs[0x500 / 2 + 1], 0x1002);
	h_eq_u64("index 10 a l'octet 0x514", regs[0x500 / 2 + 10], 0x100b);
	h_eq_u64("relecture", dispi_rd(&d, 4), 0x1005);
	h_eq_i64("aucune ecriture hors 0x500-0x515",
		fake_regs_foreign(regs, 2048, 0xa5a5), 0);
}

int	main(void)
{
	h_begin("a13/dispi_detect");
	h_run("detect partitions de l'identifiant et de la vram", detect_table);
	h_run("detect sans operations", detect_no_ops);
	h_run("detect correspondance PCI", pci_match);
	h_run("detect disposition MMIO", mmio_layout);
	return (h_end());
}
