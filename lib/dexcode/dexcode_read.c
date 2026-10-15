#include "dexcode_int.h"

uint32_t	dexcode_unit(const uint8_t *p, uint64_t i)
{
	return ((uint32_t)p[2 * i] | ((uint32_t)p[2 * i + 1] << 8));
}

uint32_t	dexcode_field(const uint8_t *p, uint8_t src)
{
	if (src == DS_AA)
		return (dexcode_unit(p, 0) >> 8);
	if (src == DS_A4)
		return ((dexcode_unit(p, 0) >> 8) & 15);
	if (src == DS_B4)
		return (dexcode_unit(p, 0) >> 12);
	if (src == DS_W1)
		return (dexcode_unit(p, 1));
	if (src == DS_W2)
		return (dexcode_unit(p, 2));
	if (src == DS_W3)
		return (dexcode_unit(p, 3));
	if (src == DS_W1L)
		return (dexcode_unit(p, 1) & 0xff);
	if (src == DS_W1H)
		return (dexcode_unit(p, 1) >> 8);
	if (src == DS_L32)
		return (dexcode_unit(p, 1) | (dexcode_unit(p, 2) << 16));
	return (0);
}

static int64_t	sext(uint32_t v, uint32_t sign)
{
	return ((int64_t)(v ^ sign) - (int64_t)sign);
}

int64_t	dexcode_literal(const uint8_t *p, uint8_t src)
{
	uint64_t	hi;

	if (src == DL_B4)
		return (sext(dexcode_field(p, DS_B4), 8));
	if (src == DL_AA)
		return (sext(dexcode_field(p, DS_AA), 0x80));
	if (src == DL_W1)
		return (sext(dexcode_field(p, DS_W1), 0x8000));
	if (src == DL_W1H)
		return (sext(dexcode_field(p, DS_W1H), 0x80));
	if (src == DL_32)
		return (sext(dexcode_field(p, DS_L32), 0x80000000u));
	if (src != DL_64)
		return (0);
	hi = dexcode_unit(p, 3) | ((uint64_t)dexcode_unit(p, 4) << 16);
	return ((int64_t)(dexcode_field(p, DS_L32) | (hi << 32)));
}

int32_t	dexcode_rd32(const uint8_t *p)
{
	return ((int32_t)sext(dexcode_unit(p, 0) | (dexcode_unit(p, 1) << 16),
			0x80000000u));
}
