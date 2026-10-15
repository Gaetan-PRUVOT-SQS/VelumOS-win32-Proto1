#include "acpi_int.h"
#include "velum/err.h"

const struct s_acpi_sdt	*acpi_set_find(const t_acpi_set *set,
	const char *sig, uint32_t index)
{
	uint32_t	k;

	k = 0;
	while (sig && k < set->count)
	{
		if (acpi_sig_eq(set->tab[k].hdr->sig, sig))
		{
			if (index == 0)
				return (set->tab[k].hdr);
			index--;
		}
		k++;
	}
	return (NULL);
}

int	acpi_parse_hpet(const struct s_acpi_sdt *t, t_acpi_info *i)
{
	uint64_t	addr;

	if (t->length < HPET_TABLE_MIN
		|| acpi_rd(t, HPET_TABLE_ADDR, 1) != GAS_SPACE_MEM)
		return (E_INVAL);
	addr = acpi_rd(t, HPET_TABLE_ADDR + GAS_ADDR, 8);
	if (!addr)
		return (E_INVAL);
	i->hpet_phys = addr;
	return (E_OK);
}

int	acpi_parse_mcfg(const struct s_acpi_sdt *t, t_acpi_info *i)
{
	const uint8_t	*e;
	uint32_t		n;
	uint32_t		k;
	t_mcfg_entry	*m;

	if (t->length < MCFG_ENTRIES)
		return (E_INVAL);
	n = (t->length - MCFG_ENTRIES) / MCFG_ENTRY_LEN;
	k = 0;
	while (k < n && i->nmcfg < ACPI_MAX_MCFG)
	{
		e = (const uint8_t *)t + MCFG_ENTRIES + k * MCFG_ENTRY_LEN;
		m = &i->mcfg[i->nmcfg];
		m->base = acpi_rd(e, 0, 8);
		m->segment = (uint16_t)acpi_rd(e, 8, 2);
		m->bus_start = e[10];
		m->bus_end = e[11];
		m->reserved = 0;
		if (m->base && m->bus_end >= m->bus_start)
			i->nmcfg++;
		k++;
	}
	return (E_OK);
}

static int	s5_lookup(const t_acpi_set *set, t_acpi_info *i)
{
	const struct s_acpi_sdt	*t;
	uint32_t				k;

	t = acpi_set_find(set, "DSDT", 0);
	k = 0;
	while (t)
	{
		if (acpi_find_s5((const uint8_t *)t + ACPI_HDR_LEN,
				t->length - ACPI_HDR_LEN, &i->slp_typ_a, &i->slp_typ_b) == E_OK)
			return (E_OK);
		t = acpi_set_find(set, "SSDT", k);
		k++;
	}
	return (E_NOENT);
}

int	acpi_parse_all(const t_acpi_set *set, t_acpi_info *i, t_acpi_extra *x)
{
	const struct s_acpi_sdt	*t;

	t = acpi_set_find(set, "APIC", 0);
	if (t)
		acpi_parse_madt(t, i, x);
	t = acpi_set_find(set, "FACP", 0);
	if (t)
		acpi_parse_fadt(t, i, x);
	t = acpi_set_find(set, "HPET", 0);
	if (t)
		acpi_parse_hpet(t, i);
	t = acpi_set_find(set, "MCFG", 0);
	if (t)
		acpi_parse_mcfg(t, i);
	x->have_s5 = (s5_lookup(set, i) == E_OK);
	i->poweroff_ok = (x->have_s5 && i->pm1a_cnt != 0);
	return (E_OK);
}
