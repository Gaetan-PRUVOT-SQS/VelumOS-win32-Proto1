#include "fake.h"

static double	root5(double v)
{
	double	g;
	int		i;

	g = 1.0;
	i = 0;
	while (i < 60)
	{
		g = (4.0 * g + v / (g * g * g * g)) / 5.0;
		i++;
	}
	return (g);
}

static double	lin(uint32_t ch)
{
	double	c;

	c = (double)ch / 255.0;
	if (c <= 0.03928)
		return (c / 12.92);
	c = (c + 0.055) / 1.055;
	return (c * c * root5(c * c));
}

static double	lum(t_color c)
{
	return (0.2126 * lin((c >> 16) & 0xff) + 0.7152 * lin((c >> 8) & 0xff)
		+ 0.0722 * lin(c & 0xff));
}

double	fake_contrast(t_color a, t_color b)
{
	double	la;
	double	lb;

	la = lum(a);
	lb = lum(b);
	if (la < lb)
		return ((lb + 0.05) / (la + 0.05));
	return ((la + 0.05) / (lb + 0.05));
}
