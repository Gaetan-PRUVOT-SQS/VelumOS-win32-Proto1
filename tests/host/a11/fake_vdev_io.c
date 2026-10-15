#include "fake.h"
#include "virtio.h"

uint8_t	vio_rd8(volatile uint8_t *base, uint32_t off)
{
	t_fkdev		*f;
	uint32_t	o;

	f = fk_by_addr(base + off, &o);
	return ((uint8_t)fk_region_rd(f, o, 1));
}

uint16_t	vio_rd16(volatile uint8_t *base, uint32_t off)
{
	t_fkdev		*f;
	uint32_t	o;

	f = fk_by_addr(base + off, &o);
	return ((uint16_t)fk_region_rd(f, o, 2));
}

uint32_t	vio_rd32(volatile uint8_t *base, uint32_t off)
{
	t_fkdev		*f;
	uint32_t	o;

	f = fk_by_addr(base + off, &o);
	return (fk_region_rd(f, o, 4));
}

void	vio_wr8(volatile uint8_t *base, uint32_t off, uint8_t v)
{
	t_fkdev		*f;
	uint32_t	o;

	f = fk_by_addr(base + off, &o);
	fk_region_wr(f, o, v);
}

void	vio_wr16(volatile uint8_t *base, uint32_t off, uint16_t v)
{
	t_fkdev		*f;
	uint32_t	o;

	f = fk_by_addr(base + off, &o);
	fk_region_wr(f, o, v);
}
