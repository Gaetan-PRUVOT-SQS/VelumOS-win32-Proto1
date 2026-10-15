#include <string.h>
#include "harness.h"
#include "fake_cpuid.h"
#include "cpu_int.h"

static void	cpuid_signatures(void)
{
	t_cpuid_raw	raw;
	t_cpufeat	f;

	memset(&raw, 0, sizeof(raw));
	raw.r[CPUID_L1][R_EAX] = 0x00000f29;
	cpuid_decode(&raw, &f);
	h_eq_u64("famille 0xf sans extension", f.family, 15);
	h_eq_u64("modele 0xf", f.model, 2);
	raw.r[CPUID_L1][R_EAX] = 0x0fff0f00;
	cpuid_decode(&raw, &f);
	h_eq_u64("famille etendue max", f.family, 15 + 255);
	h_eq_u64("modele etendu max", f.model, 0xf0);
	raw.r[CPUID_L1][R_EAX] = 0x000f0543;
	cpuid_decode(&raw, &f);
	h_eq_u64("famille 5 ignore le modele etendu", f.model, 4);
	h_eq_u64("famille 5", f.family, 5);
	h_true(!cpuid_bit(&raw, -1, 0, 0) && !cpuid_bit(&raw, CPUID_NLEAVES, 0, 0)
		&& !cpuid_bit(&raw, 0, 4, 0) && !cpuid_bit(&raw, 0, 0, 32),
		"cpuid_bit hors bornes");
	h_true(!cpuid_bit(&raw, 0, -1, 0) && !cpuid_bit(&raw, 0, 0, -1),
		"cpuid_bit registre et bit negatifs");
}

static void	flags_limites(void)
{
	t_cpufeat	f;
	char		b[8];

	memset(&f, 0, sizeof(f));
	memset(b, 'x', sizeof(b));
	cpufeat_flags(&f, b, sizeof(b));
	h_eq_str("aucune capacite", b, "");
	f.nx = true;
	f.smep = true;
	cpufeat_flags(&f, b, 5);
	h_eq_str("troncature", b, "nx s");
	b[0] = 'z';
	cpufeat_flags(&f, b, 0);
	h_eq_u64("taille nulle sans ecriture", (uint8_t)b[0], 'z');
}

static void	marque_limites(void)
{
	t_cpuid_raw	raw;
	char		brand[49];

	fake_reset();
	fake_set(0x80000000, (const uint32_t [4]){0x80000004, 0, 0, 0});
	fake_brand("                                                ");
	cpuid_collect(&raw, fake_cpuid);
	cpuid_brand(&raw, brand);
	h_eq_str("marque toute en espaces", brand, "");
	fake_reset();
	fake_set(0x80000000, (const uint32_t [4]){0x80000004, 0, 0, 0});
	fake_brand("0123456789abcdef0123456789abcdef0123456789abcdef");
	cpuid_collect(&raw, fake_cpuid);
	cpuid_brand(&raw, brand);
	h_eq_u64("marque pleine 48 octets", strlen(brand), 48);
}

int	main(void)
{
	h_begin("a01/cpuid_bits");
	h_run("signatures limites", cpuid_signatures);
	h_run("drapeaux limites", flags_limites);
	h_run("marque limites", marque_limites);
	return (h_end());
}
