#include "harness.h"
#include "fake.h"
#include "velum/err.h"

static void	rsdp_valid(void)
{
	t_acpi_src	s;
	t_acpi_root	r;

	facpi_reset();
	s = facpi_src();
	facpi_rsdp(2, F_RSDT, F_XSDT);
	h_eq_i64("v2 xsdt rc", acpi_rsdp_parse(&s, F_RSDP, &r), E_OK);
	h_eq_u64("v2 xsdt racine", r.phys, F_XSDT);
	h_eq_u64("v2 xsdt taille entree", r.entry_size, 8);
	h_eq_u64("v2 revision", r.revision, 2);
	facpi_rsdp(0, F_RSDT, F_XSDT);
	h_eq_i64("v1 rc", acpi_rsdp_parse(&s, F_RSDP, &r), E_OK);
	h_eq_u64("v1 ignore xsdt", r.phys, F_RSDT);
	h_eq_u64("v1 taille entree", r.entry_size, 4);
	facpi_rsdp(2, F_RSDT, 0);
	h_eq_i64("v2 xsdt nul rc", acpi_rsdp_parse(&s, F_RSDP, &r), E_OK);
	h_eq_u64("v2 xsdt nul repli rsdt", r.phys, F_RSDT);
	h_eq_u64("mappages rendus", g_fmem.maps, g_fmem.unmaps);
}

static void	rsdp_corrupt(void)
{
	t_acpi_src	s;
	t_acpi_root	r;

	facpi_reset();
	s = facpi_src();
	facpi_rsdp(2, F_RSDT, F_XSDT);
	facpi_at(F_RSDP)[0] = 'X';
	h_eq_i64("signature fausse", acpi_rsdp_parse(&s, F_RSDP, &r), E_INVAL);
	facpi_rsdp(0, F_RSDT, 0);
	facpi_at(F_RSDP)[16] ^= 1;
	h_eq_i64("somme v1 fausse", acpi_rsdp_parse(&s, F_RSDP, &r), E_INVAL);
	facpi_rsdp(2, F_RSDT, F_XSDT);
	facpi_at(F_RSDP)[24] ^= 1;
	h_eq_i64("somme etendue fausse", acpi_rsdp_parse(&s, F_RSDP, &r),
		E_INVAL);
	facpi_rsdp(0, 0, 0);
	h_eq_i64("racine absente", acpi_rsdp_parse(&s, F_RSDP, &r), E_NOENT);
	h_eq_i64("adresse non mappable", acpi_rsdp_parse(&s, 0x1000, &r),
		E_FAULT);
	g_fmem.fail_after = 0;
	h_eq_i64("mappage refuse", acpi_rsdp_parse(&s, F_RSDP, &r), E_FAULT);
	h_eq_u64("mappages rendus", g_fmem.maps, g_fmem.unmaps);
}

static void	set_len(uint32_t len)
{
	uint8_t	*p;

	p = facpi_at(F_RSDP);
	facpi_put(F_RSDP, RSDP_LENGTH, len, 4);
	p[8] = 0;
	p[8] = (uint8_t)(0x100 - acpi_sum(p, ACPI_RSDP_V1_LEN));
	p[32] = 0;
	if (len >= ACPI_RSDP_V2_LEN && len <= ACPI_RSDP_MAX_LEN)
		p[32] = (uint8_t)(0x100 - acpi_sum(p, len));
}

static void	rsdp_length(void)
{
	t_acpi_src	s;
	t_acpi_root	r;

	facpi_reset();
	s = facpi_src();
	facpi_rsdp(2, F_RSDT, F_XSDT);
	set_len(35);
	h_eq_i64("longueur 35", acpi_rsdp_parse(&s, F_RSDP, &r), E_INVAL);
	set_len(36);
	h_eq_i64("longueur 36", acpi_rsdp_parse(&s, F_RSDP, &r), E_OK);
	set_len(4096);
	h_eq_i64("longueur 4096", acpi_rsdp_parse(&s, F_RSDP, &r), E_OK);
	set_len(4097);
	h_eq_i64("longueur 4097", acpi_rsdp_parse(&s, F_RSDP, &r), E_INVAL);
	set_len(0);
	h_eq_i64("longueur 0", acpi_rsdp_parse(&s, F_RSDP, &r), E_INVAL);
	h_eq_u64("mappages rendus", g_fmem.maps, g_fmem.unmaps);
}

int	main(void)
{
	h_begin("a05/acpi_rsdp");
	h_run("rsdp valide v1 v2 repli", rsdp_valid);
	h_run("rsdp corrompu", rsdp_corrupt);
	h_run("rsdp limites de longueur", rsdp_length);
	return (h_end());
}
