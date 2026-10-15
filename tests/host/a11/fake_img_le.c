#include <string.h>
#include "fake.h"

void	fk_le32(uint8_t *p, uint32_t v)
{
	p[0] = (uint8_t)v;
	p[1] = (uint8_t)(v >> 8);
	p[2] = (uint8_t)(v >> 16);
	p[3] = (uint8_t)(v >> 24);
}

void	fk_le64(uint8_t *p, uint64_t v)
{
	fk_le32(p, (uint32_t)v);
	fk_le32(p + 4, (uint32_t)(v >> 32));
}

void	fk_mbr_entry(uint8_t *s, int slot, uint8_t type, const t_fkpart *p)
{
	uint8_t	*e;

	e = s + 446 + slot * 16;
	e[0] = 0;
	e[4] = type;
	fk_le32(e + 8, (uint32_t)p->first);
	fk_le32(e + 12, (uint32_t)(p->last - p->first + 1));
	s[510] = 0x55;
	s[511] = 0xaa;
}

void	fk_pattern(uint8_t *part, uint32_t n)
{
	uint64_t	k;

	memset(part, 0, 512);
	memcpy(part, "VELUMBLKTEST", 12);
	k = 512;
	while (k < (uint64_t)n * 512)
	{
		part[k] = (uint8_t)((k / 512) * 31 + k % 512);
		k++;
	}
}
