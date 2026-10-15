#include "acpi_int.h"
#include "velum/err.h"

static uint64_t	rsdp_len(const uint8_t *p)
{
	uint64_t	len;

	if (!acpi_sig_eq(p, "RSD ") || !acpi_sig_eq(p + 4, "PTR ")
		|| acpi_sum(p, ACPI_RSDP_V1_LEN) != 0)
		return (0);
	if (p[RSDP_REVISION] < 2)
		return (ACPI_RSDP_V1_LEN);
	len = acpi_rd(p, RSDP_LENGTH, 4);
	if (len < ACPI_RSDP_V2_LEN || len > ACPI_RSDP_MAX_LEN)
		return (0);
	return (len);
}

static int	rsdp_root(const uint8_t *p, uint64_t len, t_acpi_root *out)
{
	uint64_t	xsdt;

	out->revision = p[RSDP_REVISION];
	out->phys = acpi_rd(p, RSDP_RSDT, 4);
	out->entry_size = 4;
	if (out->revision >= 2)
	{
		if (acpi_sum(p, len) != 0)
			return (E_INVAL);
		xsdt = acpi_rd(p, RSDP_XSDT, 8);
		if (xsdt)
		{
			out->phys = xsdt;
			out->entry_size = 8;
		}
	}
	if (!out->phys)
		return (E_NOENT);
	return (E_OK);
}

int	acpi_rsdp_parse(const t_acpi_src *s, uint64_t phys, t_acpi_root *out)
{
	const uint8_t	*p;
	uint64_t		len;
	uint64_t		mapped;
	int				rc;

	p = s->map(phys, ACPI_RSDP_V2_LEN, s->ctx);
	if (!p)
		return (E_FAULT);
	mapped = ACPI_RSDP_V2_LEN;
	len = rsdp_len(p);
	if (len > mapped)
	{
		s->unmap(p, mapped, s->ctx);
		p = s->map(phys, len, s->ctx);
		if (!p)
			return (E_FAULT);
		mapped = len;
	}
	rc = E_INVAL;
	if (len)
		rc = rsdp_root(p, len, out);
	s->unmap(p, mapped, s->ctx);
	return (rc);
}
