#include "harness.h"
#include "th.h"
#include "velum/err.h"

static void	axes_and_limits(void)
{
	t_accel	a;
	int32_t	oy;
	int32_t	ox;

	accel_init(&a);
	accel_set(&a, 10, 1);
	ox = th_accel_step(&a, 10, 10, &oy);
	h_eq_i64("diagonale : x", ox, 23);
	h_eq_i64("diagonale : y", oy, 23);
	a.rem_x = 0;
	a.rem_y = 0;
	ox = th_accel_step(&a, -10, -10, &oy);
	h_eq_i64("symetrie : x", ox, -23);
	h_eq_i64("symetrie : y", oy, -23);
	ox = th_accel_step(&a, 100000, -100000, &oy);
	h_true(ox <= 3 * ACCEL_INPUT_LIMIT && oy >= -3 * ACCEL_INPUT_LIMIT,
		"entree geante bornee");
}

static void	config_validation(void)
{
	t_accel	a;

	accel_init(&a);
	h_eq_i64("vitesse 0", accel_set(&a, 0, 0), E_INVAL);
	h_eq_i64("vitesse 21", accel_set(&a, 21, 0), E_INVAL);
	h_eq_i64("vitesse enorme", accel_set(&a, 0xffffffffu, 0), E_INVAL);
	h_eq_i64("acceleration 2", accel_set(&a, 10, 2), E_INVAL);
	h_eq_u64("etat inchange apres refus", a.speed, 10);
	h_eq_i64("limite basse 1", accel_set(&a, 1, 0), 0);
	h_eq_i64("limite haute 20", accel_set(&a, 20, 1), 0);
	h_eq_u64("vitesse enregistree", a.speed, 20);
	h_eq_u64("acceleration enregistree", a.enabled, 1);
}

int	main(void)
{
	h_begin("a10/mouse_cfg");
	h_run("axes, symetrie, entree geante", axes_and_limits);
	h_run("validation de la configuration", config_validation);
	return (h_end());
}
