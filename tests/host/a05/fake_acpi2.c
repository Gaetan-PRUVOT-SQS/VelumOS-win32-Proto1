#include <string.h>
#include "fake.h"

static const void	*fmap(uint64_t phys, uint64_t len, void *ctx)
{
	(void)ctx;
	if (g_fmem.fail_after == 0)
		return (NULL);
	if (g_fmem.fail_after > 0)
		g_fmem.fail_after--;
	if (phys < FAKE_BASE || len > FAKE_SIZE
		|| phys - FAKE_BASE > FAKE_SIZE - len)
		return (NULL);
	g_fmem.maps++;
	return (facpi_at(phys));
}

static void	funmap(const void *v, uint64_t len, void *ctx)
{
	(void)v;
	(void)len;
	(void)ctx;
	g_fmem.unmaps++;
}

t_acpi_src	facpi_src(void)
{
	t_acpi_src	s;

	s.map = fmap;
	s.unmap = funmap;
	s.ctx = NULL;
	return (s);
}

void	facpi_rsdp(uint8_t rev, uint64_t rsdt, uint64_t xsdt)
{
	uint8_t	*p;

	p = facpi_at(F_RSDP);
	memset(p, 0, ACPI_RSDP_V2_LEN);
	memcpy(p, "RSD PTR ", 8);
	memcpy(p + 9, "VELUM ", 6);
	p[RSDP_REVISION] = rev;
	facpi_put(F_RSDP, RSDP_RSDT, rsdt, 4);
	facpi_put(F_RSDP, RSDP_LENGTH, ACPI_RSDP_V2_LEN, 4);
	facpi_put(F_RSDP, RSDP_XSDT, xsdt, 8);
	p[8] = (uint8_t)(0x100 - acpi_sum(p, ACPI_RSDP_V1_LEN));
	p[32] = (uint8_t)(0x100 - acpi_sum(p, ACPI_RSDP_V2_LEN));
}

void	facpi_root(uint64_t phys, uint32_t esize, const uint64_t *tabs,
	uint32_t n)
{
	uint32_t	k;
	const char	*sig;

	sig = "RSDT";
	if (esize == 8)
		sig = "XSDT";
	facpi_hdr(phys, sig, ACPI_HDR_LEN + n * esize, 1);
	k = 0;
	while (k < n)
	{
		facpi_put(phys, ACPI_HDR_LEN + k * esize, tabs[k], esize);
		k++;
	}
	facpi_fix(phys);
}
