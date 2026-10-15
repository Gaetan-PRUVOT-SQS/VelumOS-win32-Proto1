#ifndef DISPI_H
# define DISPI_H

# include <stdbool.h>
# include <stdint.h>
# include "velum/abi/abi_types.h"
# include "velum/pci.h"

# define DISPI_PCI_VENDOR 0x1234
# define DISPI_PCI_DEVICE 0x1111
# define DISPI_PORT_INDEX 0x1ce
# define DISPI_PORT_DATA 0x1cf
# define DISPI_MMIO_BAR 2
# define DISPI_MMIO_REGS 0x500

# define DISPI_INDEX_ID 0x0
# define DISPI_INDEX_XRES 0x1
# define DISPI_INDEX_YRES 0x2
# define DISPI_INDEX_BPP 0x3
# define DISPI_INDEX_ENABLE 0x4
# define DISPI_INDEX_VIRT_WIDTH 0x6
# define DISPI_INDEX_VRAM64K 0xa

# define DISPI_ID_LFB 45250
# define DISPI_ID_WRITE 45253
# define DISPI_ID_LIMIT 45263
# define DISPI_OFF 0x00
# define DISPI_ON 0x41
# define DISPI_VRAM_UNIT 65536ull
# define DISPI_BYTES_PIXEL 4

typedef struct s_dispi_ops
{
	uint16_t	(*read)(void *ctx, uint16_t index);
	void		(*write)(void *ctx, uint16_t index, uint16_t val);
	void		*ctx;
}	t_dispi_ops;

typedef struct s_dispi
{
	t_dispi_ops	ops;
	uint64_t	vram;
	uint64_t	bar_bytes;
	uint32_t	boot_w;
	uint32_t	boot_h;
	bool		opened;
	bool		ready;
}	t_dispi;

typedef struct s_dispi_geom
{
	uint32_t	width;
	uint32_t	height;
	uint32_t	pitch;
}	t_dispi_geom;

typedef struct s_modelist
{
	t_dispmode	*out;
	uint32_t	max;
	uint32_t	cur_w;
	uint32_t	cur_h;
	uint32_t	total;
	uint32_t	written;
}	t_modelist;

int			dispi_hw_open(t_dispi *d, uint64_t fb_phys);
int			dispi_detect(t_dispi *d);
bool		dispi_pci_match(const t_pcidev *dev, uint64_t fb_phys);
bool		dispi_mode_ok(const t_dispi *d, uint32_t w, uint32_t h);
uint32_t	dispi_list(const t_dispi *d, t_modelist *l);
int			dispi_program(t_dispi *d, uint32_t w, uint32_t h, t_dispi_geom *g);
void		dispi_mmio_bind(t_dispi *d, void *bar);
void		dispi_io_bind(t_dispi *d);

static inline uint16_t	dispi_rd(const t_dispi *d, uint16_t index)
{
	return (d->ops.read(d->ops.ctx, index));
}

static inline void	dispi_wr(const t_dispi *d, uint16_t idx, uint16_t val)
{
	d->ops.write(d->ops.ctx, idx, val);
}

#endif
