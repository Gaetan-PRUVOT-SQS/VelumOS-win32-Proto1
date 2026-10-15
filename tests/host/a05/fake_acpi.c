#include <string.h>
#include "fake.h"

t_fakemem	g_fmem;

void	facpi_reset(void)
{
	memset(&g_fmem, 0, sizeof(g_fmem));
	g_fmem.fail_after = -1;
}

uint8_t	*facpi_at(uint64_t phys)
{
	return (&g_fmem.mem[phys - FAKE_BASE]);
}

void	facpi_put(uint64_t phys, uint32_t off, uint64_t v, uint32_t size)
{
	uint8_t		*p;
	uint32_t	k;

	p = facpi_at(phys) + off;
	k = 0;
	while (k < size)
	{
		p[k] = (uint8_t)(v >> (8 * k));
		k++;
	}
}

void	facpi_hdr(uint64_t phys, const char *sig, uint32_t len, uint8_t rev)
{
	uint8_t	*p;

	p = facpi_at(phys);
	memcpy(p, sig, 4);
	facpi_put(phys, 4, len, 4);
	p[8] = rev;
	memcpy(p + 10, "VELUM ", 6);
	memcpy(p + 16, "TESTTABL", 8);
}

void	facpi_fix(uint64_t phys)
{
	uint8_t		*p;
	uint32_t	len;

	p = facpi_at(phys);
	len = (uint32_t)acpi_rd(p, 4, 4);
	p[9] = 0;
	p[9] = (uint8_t)(0x100 - acpi_sum(p, len));
}
