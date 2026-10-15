#ifndef PCI_INT_H
# define PCI_INT_H

# include <stdbool.h>
# include <stddef.h>
# include <stdint.h>
# include "velum/acpi.h"
# include "velum/pci.h"
# include "velum/sync.h"

# define PCI_DEVS_PER_BUS 32
# define PCI_FNS_PER_DEV 8
# define PCI_LEGACY_CFG_SIZE 256
# define PCI_ECAM_CFG_SIZE 4096
# define PCI_ECAM_BUS_LEN 0x100000ull
# define PCI_ECAM_MAP_MAX 64
# define PCI_MAX_DEPTH 32
# define PCI_MAX_CAPS 48
# define PCI_MAP_MAX 0x10000000ull
# define PCI_REG_ID 0x00
# define PCI_REG_COMMAND 0x04
# define PCI_REG_STATUS 0x06
# define PCI_REG_CLASS 0x08
# define PCI_REG_HEADER 0x0e
# define PCI_REG_BAR0 0x10
# define PCI_REG_BUSES 0x18
# define PCI_REG_SUBSYS 0x2c
# define PCI_REG_CAP_PTR 0x34
# define PCI_REG_IRQ 0x3c
# define PCI_STATUS_CAPS 0x0010
# define PCI_CMD_DECODE 0x0003
# define PCI_CMD_INTX_OFF 0x0400
# define PCI_HDR_MULTIFN 0x80
# define PCI_HDR_TYPE_MASK 0x7f
# define PCI_HDR_NORMAL 0
# define PCI_HDR_BRIDGE 1
# define PCI_CLASS_BRIDGE 0x06
# define PCI_SUBCLASS_PCI_BRIDGE 0x04
# define PCI_CAP_MSI 0x05
# define PCI_CAP_PCIE 0x10
# define PCI_CAP_MSIX 0x11
# define PCI_MSI_CTRL 2
# define PCI_MSI_ADDR 4
# define PCI_MSI_ENABLE 0x0001
# define PCI_MSI_MME_MASK 0x0070
# define PCI_MSI_64BIT 0x0080
# define PCI_MSI_PVM 0x0100
# define PCI_MSI_ADDR_BASE 0xfee00000u
# define PCI_VEC_MIN 0x10
# define PCI_VEC_MAX 0xfe
# define PCI_IRQ_NONE 0
# define PCI_IRQ_MSI 1
# define PCI_IRQ_INTX 2
# define PCI_IRQ_LINE_NONE 0xff
# define PCI_BACKEND_LEGACY 0
# define PCI_BACKEND_ECAM 1

typedef struct s_pciloc
{
	uint16_t	seg;
	uint8_t		bus;
	uint8_t		dev;
	uint8_t		fn;
}	t_pciloc;

typedef struct s_pciops
{
	uint32_t	(*read)(const t_pciloc *l, uint16_t o, uint8_t w);
	void		(*write)(const t_pciloc *l, uint16_t o, uint8_t w, uint32_t v);
	bool		(*bus_ok)(uint16_t seg, uint8_t bus);
	uint16_t	cfg_size;
}	t_pciops;

typedef struct s_pciclass
{
	uint8_t		cls;
	uint8_t		sub;
	const char	*name;
}	t_pciclass;

typedef struct s_pcibar
{
	uint64_t	base;
	uint64_t	size;
	uint8_t		flags;
	uint8_t		slots;
}	t_pcibar;

typedef struct s_msimsg
{
	uint32_t	addr_lo;
	uint32_t	addr_hi;
	uint16_t	data;
}	t_msimsg;

typedef struct s_pcipriv
{
	uint16_t	msi_off;
	uint16_t	msix_off;
	uint16_t	pcie_off;
	uint8_t		header_type;
	uint8_t		irq_mode;
	int			vector;
	uint32_t	gsi;
	t_irqfn		fn;
}	t_pcipriv;

typedef struct s_ecam_map
{
	uint16_t			seg;
	uint8_t				bus;
	void				*base;
}	t_ecam_map;

typedef struct s_ecam
{
	t_spinlock		lock;
	uint32_t		nwin;
	uint32_t		nmap;
	t_mcfg_entry	win[ACPI_MAX_MCFG];
	t_ecam_map		map[PCI_ECAM_MAP_MAX];
}	t_ecam;

typedef struct s_pcistate
{
	t_spinlock		lock;
	t_pciops		ops;
	t_ecam			ecam;
	bool			has_ops;
	uint32_t		count;
	uint32_t		overflow;
	t_pcidev		devs[PCI_MAX_DEVS];
	t_pcipriv		priv[PCI_MAX_DEVS];
}	t_pcistate;

typedef struct s_pciscan
{
	uint16_t	seg;
	uint8_t		seen[32];
}	t_pciscan;

t_pcistate	*pci_state(void);
void		pci_state_reset(void);
void		pcicfg_set_ops(const t_pciops *ops);
uint32_t	pcicfg_read(const t_pciloc *l, uint16_t o, uint8_t w);
int			pcicfg_write(const t_pciloc *l, uint16_t o, uint8_t w, uint32_t v);
int			pcicfg_probe(const t_pciloc *l, uint32_t *id);
void		pci_ecam_ops(t_pciops *out);
int			pci_ecam_init(const t_mcfg_entry *win, uint32_t n);
uint32_t	pci_ecam_offset(const t_pciloc *l, uint16_t off);
int			pci_ecam_window(uint16_t seg, uint8_t bus);
void		*pci_ecam_bus_base(uint16_t seg, uint8_t bus);
void		pci_legacy_ops(t_pciops *out);
uint32_t	pci_legacy_addr(const t_pciloc *l, uint16_t off);
int			pcicap_next(const t_pciloc *l, uint8_t id, uint16_t prev);
int			pcibar_scan(const t_pciloc *l, uint8_t nbars, t_pcidev *d);
void		pcibar_decode(const uint32_t *v, const uint32_t *h, t_pcibar *b);
int			pcibar_probe(const t_pciloc *l, uint8_t i, uint8_t n, t_pcibar *b);
int			pcidev_add(const t_pciloc *l, uint8_t header_type);
int			pcidev_find_loc(const t_pciloc *l);
int			pcidev_index(const t_pcidev *d);
void		pcidev_loc(const t_pcidev *d, t_pciloc *l);
void		pci_scan_root(uint16_t seg, uint8_t bus);
void		pci_scan_all(int backend);
int			pci_msi_encode(uint32_t apic, int vec, t_msimsg *msg);
int			pcimsi_enable(const t_pciloc *l, uint16_t cap, const t_msimsg *m);
void		pcimsi_disable(const t_pciloc *l, uint16_t cap);
const char	*pci_class_name(uint8_t cls, uint8_t sub);
const char	*pci_bar_kind(uint8_t flags);
const char	*pci_bar_pref(uint8_t flags);
const char	*pci_backend_name(int backend);
void		pci_log_devices(int backend);
int			pci_selftest_devices(void);
int			pci_selftest_edu(void);
int			pci_use_backends(const t_acpi_info *ai);

#endif
