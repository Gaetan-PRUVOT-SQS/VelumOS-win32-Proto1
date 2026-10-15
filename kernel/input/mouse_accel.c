#include "mouse.h"
#include "velum/err.h"

void	accel_init(t_accel *a)
{
	a->rem_x = 0;
	a->rem_y = 0;
	a->speed = ACCEL_SPEED_DEFAULT;
	a->enabled = 0;
}

static uint32_t	magnitude(int32_t v)
{
	if (v < 0)
		return ((uint32_t)(-(int64_t)v));
	return ((uint32_t)v);
}

uint32_t	accel_gain(const t_accel *a, int32_t dx, int32_t dy)
{
	uint32_t	ax;
	uint32_t	ay;
	uint32_t	m;
	uint32_t	factor;

	ax = magnitude(dx);
	ay = magnitude(dy);
	m = ax + ay / 2;
	if (ay > ax)
		m = ay + ax / 2;
	factor = ACCEL_UNIT;
	if (a->enabled && m >= ACCEL_TOP)
		factor = ACCEL_FACTOR_MAX;
	else if (a->enabled && m > ACCEL_KNEE)
		factor = ACCEL_UNIT + (m - ACCEL_KNEE) * ACCEL_SLOPE;
	return (a->speed * ACCEL_UNIT / ACCEL_SPEED_BASE * factor / ACCEL_UNIT);
}

static int32_t	axis_scale(int32_t d, uint32_t gain, int32_t *rem)
{
	int64_t	v;
	int64_t	out;

	if (d > ACCEL_INPUT_LIMIT)
		d = ACCEL_INPUT_LIMIT;
	if (d < -ACCEL_INPUT_LIMIT)
		d = -ACCEL_INPUT_LIMIT;
	v = (int64_t)d * gain + *rem;
	out = v / ACCEL_UNIT;
	*rem = (int32_t)(v - out * ACCEL_UNIT);
	return ((int32_t)out);
}

void	accel_apply(t_accel *a, int32_t *dx, int32_t *dy)
{
	uint32_t	gain;

	gain = accel_gain(a, *dx, *dy);
	*dx = axis_scale(*dx, gain, &a->rem_x);
	*dy = axis_scale(*dy, gain, &a->rem_y);
}
