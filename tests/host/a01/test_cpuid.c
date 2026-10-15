#include <string.h>
#include "harness.h"
#include "fake_cpuid.h"
#include "cpu_int.h"

static void	load_intel(uint32_t max_basic, uint32_t max_ext)
{
	const uint32_t	l0[4] = {max_basic, 0x756e6547, 0x6c65746e, 0x49656e69};
	const uint32_t	l1[4] = {0x000906ea, 0x02100800, 0xc7a20000, 0x07016000};
	const uint32_t	l7[4] = {0, 0x00140081, 0x4, 0};
	const uint32_t	e0[4] = {max_ext, 0, 0, 0};
	const uint32_t	e1[4] = {0, 0, 0, 0x00100000};

	fake_reset();
	fake_set(0, l0);
	fake_set(1, l1);
	fake_set(7, l7);
	fake_set(0x80000000, e0);
	fake_set(0x80000001, e1);
	fake_set(0x80000007, (const uint32_t [4]){0, 0, 0, 0x100});
	fake_set(0x80000008, (const uint32_t [4]){0x3027, 0, 0, 0});
	fake_brand("       Intel(R) Core(TM) i7-8700 CPU @ 3.20GHz");
}

static void	cpuid_intel(void)
{
	t_cpuid_raw	raw;
	t_cpufeat	f;
	char		flags[160];

	load_intel(0x16, 0x80000008);
	cpuid_collect(&raw, fake_cpuid);
	cpuid_decode(&raw, &f);
	h_eq_str("vendeur intel", f.vendor, "GenuineIntel");
	h_eq_str("marque sans espaces de tete",
		f.brand, "Intel(R) Core(TM) i7-8700 CPU @ 3.20GHz");
	h_eq_u64("famille 6", f.family, 6);
	h_eq_u64("modele etendu 0x9e", f.model, 0x9e);
	h_eq_u64("stepping", f.stepping, 10);
	h_eq_u64("bits physiques", f.phys_bits, 39);
	h_eq_u64("bits virtuels", f.virt_bits, 48);
	cpufeat_flags(&f, flags, sizeof(flags));
	h_eq_str("toutes les capacites", flags, "nx smep smap umip pcid fsgsbase "
		"xsave rdrand rdseed x2apic tscdl invtsc pat pge sse2 hv");
	h_true(!fake_called(0x80000009), "aucune feuille non demandee");
}

static void	cpuid_amd(void)
{
	t_cpuid_raw	raw;
	t_cpufeat	f;
	char		flags[160];

	fake_reset();
	fake_set(0, (const uint32_t [4]){0x10, 0x68747541, 0x444d4163,
		0x69746e65});
	fake_set(1, (const uint32_t [4]){0x00830f10, 0, 0x04000000, 0x04000000});
	fake_set(7, (const uint32_t [4]){0, 0, 0, 0});
	fake_set(0x80000000, (const uint32_t [4]){0x80000020, 0, 0, 0});
	fake_set(0x80000001, (const uint32_t [4]){0, 0, 0, 0x00100000});
	fake_set(0x80000007, (const uint32_t [4]){0, 0, 0, 0});
	fake_set(0x80000008, (const uint32_t [4]){0x3030, 0, 0, 0});
	fake_brand("AMD Ryzen 7 3700X 8-Core Processor              ");
	cpuid_collect(&raw, fake_cpuid);
	cpuid_decode(&raw, &f);
	h_eq_str("vendeur amd", f.vendor, "AuthenticAMD");
	h_eq_str("marque espaces de fin", f.brand,
		"AMD Ryzen 7 3700X 8-Core Processor");
	h_eq_u64("famille 0x17", f.family, 0x17);
	h_eq_u64("modele 0x31", f.model, 0x31);
	h_eq_u64("bits physiques amd", f.phys_bits, 48);
	cpufeat_flags(&f, flags, sizeof(flags));
	h_eq_str("capacites amd", flags, "nx xsave sse2");
}

static void	cpuid_feuilles_absentes(void)
{
	t_cpuid_raw	raw;
	t_cpufeat	f;

	load_intel(1, 0x80000000);
	cpuid_collect(&raw, fake_cpuid);
	cpuid_decode(&raw, &f);
	h_true(!fake_called(7), "feuille 7 au-dela du max non lue");
	h_true(!fake_called(0x80000001), "feuille etendue au-dela du max");
	h_true(!f.smep && !f.smap && !f.umip && !f.nx, "bits absents a zero");
	h_eq_str("marque vide", f.brand, "");
	h_eq_u64("bits physiques par defaut", f.phys_bits, 36);
	h_eq_u64("bits virtuels par defaut", f.virt_bits, 48);
	load_intel(0x16, 0x00000004);
	cpuid_collect(&raw, fake_cpuid);
	h_eq_u64("max etendu invalide efface", raw.r[CPUID_E0][R_EAX], 0);
	h_true(!fake_called(0x80000002), "pas de marque si max etendu invalide");
	load_intel(0x16, 0x80010000);
	cpuid_collect(&raw, fake_cpuid);
	h_eq_u64("max etendu au-dela de 0x8000ffff", raw.r[CPUID_E0][R_EAX], 0);
}

int	main(void)
{
	h_begin("a01/cpuid");
	h_run("intel", cpuid_intel);
	h_run("amd", cpuid_amd);
	h_run("feuilles absentes", cpuid_feuilles_absentes);
	return (h_end());
}
