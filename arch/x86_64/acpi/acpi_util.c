#include "acpi_int.h"
#include "velum/err.h"

uint64_t	acpi_rd(const void *base, uint32_t off, uint32_t size)
{
	const uint8_t	*p;
	uint64_t		v;

	p = (const uint8_t *)base + off;
	v = 0;
	while (size > 0)
	{
		size--;
		v = (v << 8) | p[size];
	}
	return (v);
}

uint8_t	acpi_sum(const void *p, uint64_t len)
{
	const uint8_t	*b;
	uint8_t			sum;

	b = p;
	sum = 0;
	while (len > 0)
	{
		sum = (uint8_t)(sum + *b);
		b++;
		len--;
	}
	return (sum);
}

bool	acpi_sig_eq(const void *a, const char *sig)
{
	const char	*s;

	s = a;
	return (s[0] == sig[0] && s[1] == sig[1] && s[2] == sig[2]
		&& s[3] == sig[3]);
}

int	acpi_set_add(t_acpi_set *set, const struct s_acpi_sdt *t, uint64_t phys)
{
	uint32_t	i;

	i = 0;
	while (i < set->count)
	{
		if (set->tab[i].phys == phys)
			return (E_EXIST);
		i++;
	}
	if (set->count >= ACPI_TABLES_MAX)
	{
		set->dropped++;
		return (E_NOMEM);
	}
	set->tab[set->count].hdr = t;
	set->tab[set->count].phys = phys;
	set->count++;
	return (E_OK);
}

void	madt_add_cpu(uint32_t apic_id, uint32_t uid, t_acpi_info *i,
	t_acpi_extra *x)
{
	uint32_t	k;

	k = 0;
	while (k < i->ncpus)
	{
		if (i->lapic_id[k] == apic_id)
			return ;
		k++;
	}
	if (i->ncpus >= ACPI_MAX_CPUS)
	{
		x->cpus_ignored++;
		return ;
	}
	i->lapic_id[i->ncpus] = apic_id;
	x->lapic_uid[i->ncpus] = uid;
	i->ncpus++;
}
