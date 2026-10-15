#include "fakes.h"
#include "dispi.h"
#include "velum/err.h"

void	fake_dispi_bind(t_dispi *d)
{
	memset(d, 0, sizeof(*d));
	d->ops.read = fake_dispi_read;
	d->ops.write = fake_dispi_write;
	d->ops.ctx = NULL;
	d->bar_bytes = g_fd.bar_bytes;
}

int	dispi_hw_open(t_dispi *d, uint64_t fb_phys)
{
	(void)fb_phys;
	g_fd.open_calls++;
	if (d->opened)
		return (E_OK);
	if (!g_fd.present)
		return (E_NODEV);
	d->ops.read = fake_dispi_read;
	d->ops.write = fake_dispi_write;
	d->ops.ctx = NULL;
	d->bar_bytes = g_fd.bar_bytes;
	d->opened = true;
	return (E_OK);
}

void	fake_dispi_ready(t_dispi *d)
{
	fake_dispi_reset();
	fake_dispi_bind(d);
	d->boot_w = 1024;
	d->boot_h = 768;
	h_eq_i64("detection de l'appareil", dispi_detect(d), E_OK);
	g_fd.nlog = 0;
}

void	fake_regs_fill(uint16_t *regs, size_t n, uint16_t v)
{
	size_t	i;

	i = 0;
	while (i < n)
		regs[i++] = v;
}

int	fake_regs_foreign(const uint16_t *regs, size_t n, uint16_t v)
{
	size_t	i;
	int		bad;

	i = 0;
	bad = 0;
	while (i < n)
	{
		if (i < 0x500 / 2 || i > 0x500 / 2 + DISPI_INDEX_VRAM64K)
			bad += (regs[i] != v);
		i++;
	}
	return (bad);
}
