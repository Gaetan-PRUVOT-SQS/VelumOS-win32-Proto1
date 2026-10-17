#include "velum/libk.h"
#include "sh_desk.h"

static void	box_line(const uint8_t *src, uint8_t *dst, int32_t n, int32_t step)
{
	int32_t	i;
	int32_t	j;
	int32_t	sum;

	i = 0;
	while (i < n)
	{
		sum = 0;
		j = i - SH_SHADOW_BLUR;
		while (j <= i + SH_SHADOW_BLUR)
		{
			if (j >= 0 && j < n)
				sum += src[j * step];
			j++;
		}
		dst[i * step] = (uint8_t)(sum / (2 * SH_SHADOW_BLUR + 1));
		i++;
	}
}

static void	pass(uint8_t *a, uint8_t *tmp, int32_t n, int32_t along)
{
	int32_t	k;

	k = 0;
	while (k < n)
	{
		box_line(a + k * (n / along), tmp + k * (n / along), n, along);
		k++;
	}
	memcpy(a, tmp, (size_t)n * (size_t)n);
}

void	sh_blur_alpha(uint8_t *a, uint8_t *tmp, int32_t n)
{
	int32_t	i;

	if (!a || !tmp || n < 1)
		return ;
	i = 0;
	while (i < SH_SHADOW_PASSES)
	{
		pass(a, tmp, n, 1);
		pass(a, tmp, n, n);
		i++;
	}
}
