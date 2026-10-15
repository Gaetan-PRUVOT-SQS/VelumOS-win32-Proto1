#include <stdio.h>
#include <string.h>
#include "a09_test.h"
#include "fake.h"
#include "harness.h"
#include "rng_int.h"
#include "velum/err.h"
#include "velum/random.h"

static void	boot_enregistre_les_deux_appels(void)
{
	fake_reset();
	h_eq_i64("code de retour", random_boot_init(), 0);
	h_eq_u64("deux appels enregistres", g_fake.nsys, 2);
	h_eq_u64("numero getrandom", g_fake.sys[0].num, 0x80);
	h_eq_str("nom getrandom", g_fake.sys[0].name, "getrandom");
	h_eq_u64("numero sysinfo", g_fake.sys[1].num, 0x81);
	h_eq_str("nom sysinfo", g_fake.sys[1].name, "sysinfo");
	h_true(fake_log_has("random: graine rdseed=oui rdrand=oui gigue=oui"),
		"journal des sources");
}

static void	boot_sans_materiel_alerte(void)
{
	fake_reset();
	g_fake.rdseed.mode = HW_ABSENT;
	g_fake.rdrand.mode = HW_ABSENT;
	h_eq_i64("code de retour", random_boot_init(), 0);
	h_true(fake_log_has("[warn] random: ni RDSEED ni RDRAND"),
		"alerte entropie logicielle");
	h_true(fake_log_has("rdseed=non rdrand=non gigue=oui"),
		"sources journalisees");
}

static void	boot_propage_l_echec_d_enregistrement(void)
{
	fake_reset();
	g_fake.sys_rc = E_EXIST;
	h_eq_i64("doublon d'appel systeme", random_boot_init(), E_EXIST);
}

static void	journal_sans_secret(void)
{
	char	hex[40];
	int		i;

	fake_reset();
	random_boot_init();
	i = 0;
	while (i < 8)
	{
		snprintf(hex, sizeof(hex), "%02x%02x", rng_global()->key[i],
			rng_global()->key[i + 1]);
		h_true(!fake_log_has(hex), "octets de cle journalises");
		i += 2;
	}
	h_true(!fake_log_has("key"), "mot cle absent du journal");
}

int	main(void)
{
	h_begin("a09/random_boot");
	h_run("boot/enregistrement-0x80-0x81-et-journal",
		boot_enregistre_les_deux_appels);
	h_run("boot/sans-rdseed-ni-rdrand", boot_sans_materiel_alerte);
	h_run("boot/echec-d-enregistrement", boot_propage_l_echec_d_enregistrement);
	h_run("boot/journal-sans-secret", journal_sans_secret);
	return (h_end());
}
