#include "acpi_int.h"
#include "velum/err.h"

static int	table_len(const t_acpi_src *s, uint64_t phys, uint64_t *len)
{
	const struct s_acpi_sdt	*h;

	if (!phys || phys + ACPI_HDR_LEN < phys)
		return (E_INVAL);
	h = s->map(phys, ACPI_HDR_LEN, s->ctx);
	if (!h)
		return (E_FAULT);
	*len = h->length;
	s->unmap(h, ACPI_HDR_LEN, s->ctx);
	if (*len < ACPI_HDR_LEN || *len > ACPI_TABLE_MAX_LEN
		|| phys + *len < phys)
		return (E_INVAL);
	return (E_OK);
}

const struct s_acpi_sdt	*acpi_map_table(const t_acpi_src *s, uint64_t phys,
	int *err)
{
	const struct s_acpi_sdt	*t;
	uint64_t				len;

	*err = table_len(s, phys, &len);
	if (*err < 0)
		return (NULL);
	t = s->map(phys, len, s->ctx);
	*err = E_FAULT;
	if (!t)
		return (NULL);
	*err = E_INVAL;
	if (t->length != len || acpi_sum(t, len) != 0)
	{
		s->unmap(t, len, s->ctx);
		return (NULL);
	}
	*err = E_OK;
	return (t);
}

static void	collect_one(const t_acpi_src *s, t_acpi_set *set, uint64_t phys,
	const char *want)
{
	const struct s_acpi_sdt	*t;
	int						err;

	t = acpi_map_table(s, phys, &err);
	if (!t)
	{
		set->rejected++;
		return ;
	}
	if ((want && !acpi_sig_eq(t->sig, want))
		|| acpi_set_add(set, t, phys) < 0)
	{
		if (want)
			set->rejected++;
		s->unmap(t, t->length, s->ctx);
	}
}

static const struct s_acpi_sdt	*root_open(const t_acpi_src *s,
	const t_acpi_root *root, int *err)
{
	const struct s_acpi_sdt	*r;

	r = acpi_map_table(s, root->phys, err);
	if (!r)
		return (NULL);
	if ((root->entry_size == 8 && acpi_sig_eq(r->sig, "XSDT"))
		|| (root->entry_size == 4 && acpi_sig_eq(r->sig, "RSDT")))
		return (r);
	s->unmap(r, r->length, s->ctx);
	*err = E_INVAL;
	return (NULL);
}

int	acpi_collect(const t_acpi_src *s, const t_acpi_root *root,
	t_acpi_set *set)
{
	const struct s_acpi_sdt	*r;
	const struct s_acpi_sdt	*f;
	uint32_t				n;
	uint32_t				i;
	int						err;

	r = root_open(s, root, &err);
	if (!r)
		return (err);
	set->count = 0;
	set->rejected = 0;
	set->dropped = 0;
	acpi_set_add(set, r, root->phys);
	n = (r->length - ACPI_HDR_LEN) / root->entry_size;
	i = 0;
	while (i < n)
	{
		collect_one(s, set, acpi_rd(r, ACPI_HDR_LEN + i * root->entry_size,
				root->entry_size), NULL);
		i++;
	}
	f = acpi_set_find(set, "FACP", 0);
	if (f && acpi_fadt_dsdt(f))
		collect_one(s, set, acpi_fadt_dsdt(f), "DSDT");
	return (E_OK);
}
