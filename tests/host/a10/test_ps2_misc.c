#include "harness.h"
#include "th.h"
#include "velum/err.h"

static void	mouse_without_wheel(void)
{
	th_ps2_reset();
	g_f8042.wheel_ok = 0;
	h_eq_i64("ps2_boot", ps2_boot(), 0);
	h_eq_u64("identifiant 0", g_ps2.mouse_id, 0);
	h_eq_u64("paquets de 3 octets", g_ps2.mdec.wheel, 0);
	h_eq_u64("souris prete", g_ps2.mouse_ok, 1);
}

static void	stuck_input_buffer(void)
{
	uint64_t	t0;

	th_ps2_reset();
	g_f8042.stuck_ibf = 1;
	t0 = g_ftime.now;
	h_eq_i64("IBF bloque : delai", ps2_boot(), E_TIMEOUT);
	h_true(g_ftime.now - t0 <= 60000000ull, "attente bornee a 50 ms");
	h_true(g_ftime.now - t0 >= 50000000ull, "delai reellement attendu");
	h_eq_i64("aucune IRQ", g_firq.count, 0);
}

int	main(void)
{
	h_begin("a10/ps2_misc");
	h_run("souris sans molette", mouse_without_wheel);
	h_run("tampon d'entree bloque", stuck_input_buffer);
	return (h_end());
}
