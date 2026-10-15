#include <stdio.h>
#include "display_int.h"
#include "kfix.h"

static void	fuzz_calls(void)
{
	uint8_t		buf[256];
	uint64_t	seed;
	int			i;
	int			bad;

	seed = 20261005;
	printf("a13/sys_fuzz : graine %llu\n", (unsigned long long)seed);
	fx_display(1024, 768);
	memset(buf, 0xaa, sizeof(buf));
	i = 0;
	bad = 0;
	while (i < 20000)
	{
		bad += fx_fuzz_one(&seed, buf);
		i++;
	}
	h_eq_i64("retours toujours dans l'ensemble attendu", bad, 0);
	h_eq_i64("mutex equilibre", g_fp.locks - g_fp.unlocks, 0);
	h_eq_i64("aucune erreur de verrou", g_fp.bad_locks, 0);
	h_true(fake_vmm_guards_ok(), "aucune ecriture hors des mappages");
	h_eq_i64("une seule fenetre noyau", fake_vmm_live_windows(), 1);
	h_true(fake_vmm_live_maps() <= 3, "au plus un mappage par processus");
	h_eq_u64("zone interdite d'ecriture intacte", fx_canary(buf + 128, 128), 0);
}

int	main(void)
{
	h_begin("a13/sys_fuzz");
	h_run("fuzz de 20000 appels", fuzz_calls);
	return (h_end());
}
