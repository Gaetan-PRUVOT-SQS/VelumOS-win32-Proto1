#include "acpi_int.h"
#include "velum/err.h"

static bool	s5_named(const uint8_t *aml, uint64_t i)
{
	if (i >= 1 && aml[i - 1] == AML_NAME_OP)
		return (true);
	return (i >= 2 && aml[i - 1] == AML_ROOT_CHAR
		&& aml[i - 2] == AML_NAME_OP);
}

static bool	s5_pkglen(const uint8_t *aml, uint64_t end, uint64_t *pos,
	uint64_t *pkg_end)
{
	uint64_t	start;
	uint64_t	len;
	uint32_t	n;
	uint32_t	k;

	start = *pos;
	if (start >= end)
		return (false);
	n = aml[start] >> 6;
	len = aml[start] & 0x3f;
	if (n && (aml[start] & 0x30))
		return (false);
	if (n)
		len = aml[start] & 0x0f;
	if (start + 1 + n > end)
		return (false);
	k = 0;
	while (k < n)
	{
		len |= (uint64_t)aml[start + 1 + k] << (4 + 8 * k);
		k++;
	}
	*pos = start + 1 + n;
	*pkg_end = start + len;
	return (len >= 1 + n && *pkg_end <= end);
}

static bool	s5_int(const uint8_t *aml, uint64_t end, uint64_t *pos,
	uint64_t *val)
{
	uint8_t		op;
	uint32_t	size;

	if (*pos >= end)
		return (false);
	op = aml[*pos];
	size = (op == AML_BYTE_PREFIX) + 2 * (op == AML_WORD_PREFIX)
		+ 4 * (op == AML_DWORD_PREFIX) + 8 * (op == AML_QWORD_PREFIX);
	if ((!size && op > AML_ONE_OP) || *pos + 1 + size > end)
		return (false);
	*val = op;
	if (size)
		*val = acpi_rd(aml, (uint32_t)(*pos + 1), size);
	*pos += 1 + size;
	return (true);
}

static bool	s5_package(const uint8_t *aml, uint64_t len, uint64_t pos,
	uint16_t out[2])
{
	uint64_t	end;
	uint64_t	a;
	uint64_t	b;

	if (pos >= len || aml[pos] != AML_PACKAGE_OP)
		return (false);
	pos++;
	if (!s5_pkglen(aml, len, &pos, &end) || pos >= end || aml[pos] < 2)
		return (false);
	pos++;
	if (!s5_int(aml, end, &pos, &a) || !s5_int(aml, end, &pos, &b))
		return (false);
	if (a > SLP_TYP_MAX || b > SLP_TYP_MAX)
		return (false);
	out[0] = (uint16_t)a;
	out[1] = (uint16_t)b;
	return (true);
}

int	acpi_find_s5(const uint8_t *aml, uint64_t len, uint16_t *typ_a,
	uint16_t *typ_b)
{
	uint64_t	i;
	uint16_t	v[2];

	i = 0;
	while (i + 4 <= len)
	{
		if (acpi_sig_eq(aml + i, "_S5_") && s5_named(aml, i)
			&& s5_package(aml, len, i + 4, v))
		{
			*typ_a = v[0];
			*typ_b = v[1];
			return (E_OK);
		}
		i++;
	}
	return (E_NOENT);
}
