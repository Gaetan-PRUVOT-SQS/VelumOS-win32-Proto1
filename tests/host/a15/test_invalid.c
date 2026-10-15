#include <stdint.h>
#include "harness.h"
#include "ref.h"
#include "velum/gfx.h"

static void	invalid_kind(int kind, int rounds)
{
	t_fx	a;
	t_fx	b;
	int		op;
	int		i;

	fx_open(&a, kind);
	fx_open(&b, kind);
	i = 0;
	while (i < rounds)
	{
		op = 0;
		while (op < 15)
		{
			fuzz_op(&a, &b, op);
			op++;
		}
		fx_verify(&a, "surface invalide: aucune ecriture");
		fx_verify(&b, "surface invalide: source intacte");
		i++;
	}
	fx_close(&a);
	fx_close(&b);
}

static void	stride_too_small(void)
{
	invalid_kind(7, 200);
}

static void	null_pixels(void)
{
	invalid_kind(8, 200);
}

static void	negative_size(void)
{
	invalid_kind(9, 200);
}

int	main(void)
{
	h_begin("a15/invalid");
	rng_seed(20261005);
	h_run("surface invalide: stride < w refuse", stride_too_small);
	h_run("surface invalide: px nul", null_pixels);
	h_run("surface invalide: largeur negative", negative_size);
	return (h_end());
}
