#ifndef PCI_H
# define PCI_H

# include <stdbool.h>
# include <stdint.h>
# include "irq.h"

# define PCI_MAX_DEVS 128
# define PCI_BARS 6
# define PCI_BAR_IO 0x1
# define PCI_BAR_MEM64 0x2
# define PCI_BAR_PREFETCH 0x4
# define PCI_CMD_IO 0x1
# define PCI_CMD_MEM 0x2
# define PCI_CMD_MASTER 0x4

typedef struct s_pcidev
{
	uint16_t	seg;
	uint8_t		bus;
	uint8_t		dev;
	uint8_t		fn;
	uint8_t		class_code;
	uint8_t		subclass;
	uint8_t		prog_if;
	uint8_t		revision;
	uint8_t		irq_line;
	uint8_t		irq_pin;
	uint8_t		bar_flags[PCI_BARS];
	uint16_t	vendor;
	uint16_t	device;
	uint16_t	subsys_vendor;
	uint16_t	subsys_device;
	bool		has_msi;
	bool		has_msix;
	uint64_t	bar[PCI_BARS];
	uint64_t	bar_size[PCI_BARS];
}	t_pcidev;

int				pci_boot_init(void);
uint32_t		pci_count(void);
const t_pcidev	*pci_at(uint32_t i);
const t_pcidev	*pci_find(uint16_t vendor, uint16_t device, uint32_t index);
const t_pcidev	*pci_find_class(uint8_t cls, uint8_t sub, uint32_t index);
uint32_t		pci_cfg_read32(const t_pcidev *d, uint16_t off);
void			pci_cfg_write32(const t_pcidev *d, uint16_t off, uint32_t v);
int				pci_enable(const t_pcidev *d, uint16_t cmd_bits);
void			*pci_map_bar(const t_pcidev *d, uint32_t bar);
int				pci_irq_setup(const t_pcidev *d, t_irqfn fn, void *ctx);
void			pci_irq_free(const t_pcidev *d);
uint16_t		pci_cfg_read16(const t_pcidev *d, uint16_t off);
uint8_t			pci_cfg_read8(const t_pcidev *d, uint16_t off);
void			pci_cfg_write16(const t_pcidev *d, uint16_t off, uint16_t v);
void			pci_cfg_write8(const t_pcidev *d, uint16_t off, uint8_t v);
int				pci_cap_next(const t_pcidev *d, uint8_t id, uint16_t prev);
int				pci_io_port(const t_pcidev *d, uint32_t bar);
int				pci_selftest(void);
int				pci_intx_disable(const t_pcidev *d);

#endif
