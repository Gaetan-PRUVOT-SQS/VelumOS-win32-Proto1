#include <string.h>
#include "a09_test.h"
#include "fake.h"
#include "pci_int.h"
#include "velum/vmm.h"

void	fake_ecam_window(uint8_t bus_start, uint8_t bus_end)
{
	t_mcfg_entry	w;

	fake_reset();
	fake_mmio_reset();
	pci_state_reset();
	w = (t_mcfg_entry){0xb0000000ull, 0, bus_start, bus_end, 0};
	pci_ecam_init(&w, 1);
}

void	fake_ecam_backend(void)
{
	t_pciops	ops;

	pci_ecam_ops(&ops);
	pcicfg_set_ops(&ops);
}

uint8_t	*fake_ecam_open(void)
{
	fake_ecam_window(0, 7);
	fake_ecam_backend();
	return (pci_ecam_bus_base(0, 2));
}

void	fake_ecam_poke(uint8_t bus, uint8_t dev, uint32_t vendor_device)
{
	uint8_t	*cfg;

	cfg = vmm_io_map(0xb0000000ull + ((uint64_t)bus << 20), 0x100000, 0);
	cfg += (uint32_t)dev << 15;
	memset(cfg, 0, 256);
	memcpy(cfg, &vendor_device, 4);
	cfg[0x0a] = 0;
	cfg[0x0b] = 6;
}
