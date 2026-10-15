#include "harness.h"
#include "th.h"

static void	linear_speeds(void)
{
	t_accel	a;
	int32_t	oy;
	int32_t	d;

	accel_init(&a);
	h_eq_u64("vitesse par defaut", a.speed, 10);
	d = -255;
	while (d <= 255)
	{
		h_eq_i64("vitesse 10 : identite", th_accel_step(&a, d, 0, &oy),
			d);
		d += 17;
	}
	accel_set(&a, 20, 0);
	h_eq_i64("vitesse 20 : double", th_accel_step(&a, 7, 0, &oy), 14);
	accel_set(&a, 5, 0);
	h_eq_i64("vitesse 5 : moitie", th_accel_step(&a, 8, 0, &oy), 4);
	h_eq_i64("vitesse 5 : reste conserve", th_accel_step(&a, 1, 0, &oy), 0);
	h_eq_i64("vitesse 5 : reste restitue", th_accel_step(&a, 1, 0, &oy), 1);
}

static void	remainder_carry(void)
{
	t_accel	a;
	int32_t	oy;
	int32_t	sum;
	int		i;

	accel_init(&a);
	accel_set(&a, 5, 0);
	sum = 0;
	i = 0;
	while (i < 1000)
	{
		sum += th_accel_step(&a, 1, 0, &oy);
		i++;
	}
	h_eq_i64("1000 petits mouvements a 0.5 = 500", sum, 500);
	accel_set(&a, 5, 0);
	h_eq_i64("la config remet le reste a zero",
		th_accel_step(&a, 1, 0, &oy), 0);
}

static void	accel_curve(void)
{
	t_accel	a;
	int32_t	oy;
	int32_t	prev;
	int32_t	out;
	int32_t	d;

	accel_init(&a);
	accel_set(&a, 10, 1);
	prev = 0;
	d = 0;
	while (d <= 255)
	{
		a.rem_x = 0;
		out = th_accel_step(&a, d, 0, &oy);
		h_true(out >= prev, "monotone");
		h_true(out >= d && out <= 3 * d, "borne par 1x et 3x");
		prev = out;
		d++;
	}
	a.rem_x = 0;
	h_eq_i64("sous le genou : identite", th_accel_step(&a, 4, 0, &oy), 4);
	a.rem_x = 0;
	h_eq_i64("au dessus du genou", th_accel_step(&a, 8, 0, &oy), 12);
	a.rem_x = 0;
	h_eq_i64("plafond atteint", th_accel_step(&a, 100, 0, &oy), 300);
}

int	main(void)
{
	h_begin("a10/mouse_accel");
	h_run("vitesses lineaires", linear_speeds);
	h_run("conservation du reste", remainder_carry);
	h_run("courbe monotone et plafonnee", accel_curve);
	return (h_end());
}
