#include <math.h>
#include "harness.h"
#include "d09.h"

static const double	g_pairs[][2] = {{5.5, 2.0}, {-5.5, 2.0}, {5.5, -2.0},
{1e300, 3.0}, {4.9e-324, 3.0}, {1.0, 4.9e-324}, {1e308, 1e-308},
{0.0, 1.0}, {-0.0, 1.0}, {1.0, 0.0}, {INFINITY, 1.0}, {1.0, INFINITY},
{NAN, 1.0}, {3.0, 3.0}, {-3.0, 3.0}, {2.2250738585072014e-308, 1.5e-323},
{1e17, 7.0}, {0.1, 0.03}, {1.7976931348623157e308, 4.9e-324},
{123456789.125, 0.001}, {8.0, 1e-310}
};

static uint64_t	host_d(uint32_t k, double x, double y)
{
	double	r;

	r = fmod(x, y);
	if (k == 4 && r != r)
		return (0x7ff8000000000000ull);
	if (k == 0)
		r = x + y;
	if (k == 1)
		r = x - y;
	if (k == 2)
		r = x * y;
	if (k == 3)
		r = x / y;
	return (fk_dbits(r));
}

static uint64_t	host_f(uint32_t k, float x, float y)
{
	float	r;

	r = fmodf(x, y);
	if (k == 4 && r != r)
		return (0x7fc00000u);
	if (k == 0)
		r = x + y;
	if (k == 1)
		r = x - y;
	if (k == 2)
		r = x * y;
	if (k == 3)
		r = x / y;
	return (fk_fbits(r));
}

static void	doubles_contre_l_hote(void)
{
	t_case		c;
	uint32_t	k;
	uint32_t	p;

	k = 0;
	while (k < 5 * sizeof(g_pairs) / sizeof(g_pairs[0]))
	{
		p = k / 5;
		c = (t_case){"double contre l'hote", (uint8_t)(OP_ADD_DOUBLE + k % 5),
			fk_dbits(g_pairs[p][0]), fk_dbits(g_pairs[p][1]),
			host_d(k % 5, g_pairs[p][0], g_pairs[p][1]), 0};
		fk_case3(&c);
		k++;
	}
}

static void	flottants_contre_l_hote(void)
{
	t_case		c;
	uint32_t	k;
	float		x;
	float		y;

	k = 0;
	while (k < 5 * sizeof(g_pairs) / sizeof(g_pairs[0]))
	{
		x = (float)g_pairs[k / 5][0];
		y = (float)g_pairs[k / 5][1];
		c = (t_case){"float contre l'hote", (uint8_t)(OP_ADD_FLOAT + k % 5),
			fk_fbits(x), fk_fbits(y), host_f(k % 5, x, y), 0};
		fk_case3(&c);
		k++;
	}
}

int	main(void)
{
	h_begin("d09/float");
	h_run("doubles_contre_l_hote", doubles_contre_l_hote);
	h_run("flottants_contre_l_hote", flottants_contre_l_hote);
	return (h_end());
}
