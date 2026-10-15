#include <string.h>
#include "harness.h"
#include "fake.h"
#include "velum/err.h"

static const uint8_t	g_std[] = {
	0, 8, 0, 0, 1, 0, 0, 0,
	0, 8, 1, 1, 0, 0, 0, 0,
	0, 8, 2, 2, 2, 0, 0, 0,
	0, 8, 3, 3, 1, 0, 0, 0,
	1, 12, 5, 0, 0, 0, 0xc0, 0xfe, 0, 0, 0, 0,
	2, 10, 0, 0, 2, 0, 0, 0, 0, 0,
	2, 10, 0, 9, 9, 0, 0, 0, 0x0d, 0,
	2, 10, 1, 5, 5, 0, 0, 0, 0, 0,
	4, 6, 0xff, 5, 0, 1,
	9, 16, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 7, 0, 0, 0,
	9, 16, 0, 0, 3, 0, 0, 0, 1, 0, 0, 0, 3, 0, 0, 0,
	5, 12, 0, 0, 0, 0, 0xe0, 0xfe, 1, 0, 0, 0,
	3, 8, 0, 0, 15, 0, 0, 0,
	0x10, 4, 0xaa, 0xbb,
	0, 6, 9, 9, 1, 0,
	0x0a, 12, 5, 0, 0xff, 0xff, 0xff, 0xff, 0, 0, 0, 0,
	4, 6, 1, 0, 0, 2
};
static const uint8_t	g_cut[] = {
	0, 8, 0, 0, 1, 0, 0, 0,
	0, 0, 1, 1, 1, 0, 0, 0,
	0, 8, 2, 2, 1, 0, 0, 0
};
static const uint8_t	g_tail[] = {
	0, 8, 0, 0, 1, 0, 0, 0,
	0, 8, 2, 2, 1, 0
};

static void	parse(t_acpi_info *i, t_acpi_extra *x)
{
	memset(i, 0, sizeof(*i));
	memset(x, 0, sizeof(*x));
	acpi_parse_madt((const struct s_acpi_sdt *)facpi_at(F_APIC), i, x);
}

static void	madt_types(void)
{
	t_acpi_info		i;
	t_acpi_extra	x;

	facpi_reset();
	facpi_madt(g_std, sizeof(g_std));
	parse(&i, &x);
	h_eq_u64("cpu actives et x2apic sans doublon", i.ncpus, 3);
	h_eq_u64("x2apic id 256", i.lapic_id[2], 256);
	h_eq_u64("uid x2apic", x.lapic_uid[2], 7);
	h_eq_u64("ioapic", i.nioapics, 1);
	h_eq_u64("ioapic adresse", i.ioapic[0].phys, 0xfec00000);
	h_eq_u64("ioapic id", i.ioapic[0].id, 5);
	h_eq_u64("overrides isa seulement", i.noverrides, 2);
	h_eq_u64("override 0 vers 2", i.override[0].gsi, 2);
	h_eq_u64("override 9 drapeaux", i.override[1].flags, 0x0d);
	h_eq_u64("nmi locales valides", x.nnmi, 2);
	h_eq_u64("nmi tous les cpu", x.nmi[0].uid, ACPI_UID_ALL);
	h_eq_u64("nmi lint1", x.nmi[0].lint, 1);
	h_eq_u64("nmi x2 drapeaux", x.nmi[1].flags, 5);
	h_eq_u64("source nmi", x.nmi_src_gsi[0], 15);
	h_eq_u64("adresse lapic 64 bits", i.lapic_phys, 0x1fee00000ull);
	h_eq_u64("entrees ignorees", x.madt_skipped, 3);
	h_eq_u64("drapeaux madt", x.madt_flags, MADT_PCAT_COMPAT);
}

static void	madt_broken(void)
{
	t_acpi_info		i;
	t_acpi_extra	x;

	facpi_reset();
	facpi_madt(g_cut, sizeof(g_cut));
	parse(&i, &x);
	h_eq_u64("longueur 0 arrete", i.ncpus, 1);
	h_eq_u64("longueur 0 comptee", x.madt_skipped, 1);
	facpi_madt(g_tail, sizeof(g_tail));
	parse(&i, &x);
	h_eq_u64("entree qui depasse", i.ncpus, 1);
	h_eq_u64("depassement compte", x.madt_skipped, 1);
	h_eq_u64("adresse lapic 32 bits", i.lapic_phys, 0xfee00000);
	facpi_hdr(F_APIC, "APIC", 43, 1);
	facpi_fix(F_APIC);
	memset(&i, 0, sizeof(i));
	h_eq_i64("table de 43 octets", acpi_parse_madt(
			(const struct s_acpi_sdt *)facpi_at(F_APIC), &i, &x), E_INVAL);
}

static void	madt_caps(void)
{
	t_acpi_info		i;
	t_acpi_extra	x;
	uint8_t			e[80 * 12];
	uint32_t		k;

	k = 0;
	while (k < 70)
	{
		memcpy(e + k * 8, "\0\x08\0\0\x01\0\0\0", 8);
		e[k * 8 + 3] = (uint8_t)k;
		k++;
	}
	facpi_reset();
	facpi_madt(e, 70 * 8);
	parse(&i, &x);
	h_eq_u64("plafond cpu", i.ncpus, ACPI_MAX_CPUS);
	h_eq_u64("cpu en trop", x.cpus_ignored, 70 - ACPI_MAX_CPUS);
}

int	main(void)
{
	h_begin("a05/acpi_madt");
	h_run("madt types d'entrees", madt_types);
	h_run("madt entrees cassees", madt_broken);
	h_run("madt plafonds", madt_caps);
	return (h_end());
}
