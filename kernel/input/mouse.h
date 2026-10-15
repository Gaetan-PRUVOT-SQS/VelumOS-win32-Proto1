#ifndef MOUSE_H
# define MOUSE_H

# include <stdint.h>
# include "inp_types.h"
# include "velum/abi/abi_input.h"

# define ACCEL_UNIT 256
# define ACCEL_SPEED_BASE 10
# define ACCEL_KNEE 4
# define ACCEL_TOP 20
# define ACCEL_SLOPE 32
# define ACCEL_FACTOR_MAX 768
# define ACCEL_SPEED_DEFAULT 10
# define ACCEL_INPUT_LIMIT 1024

typedef struct s_accel
{
	int32_t		rem_x;
	int32_t		rem_y;
	uint32_t	speed;
	uint32_t	enabled;
}	t_accel;

typedef struct s_mouse
{
	t_accel		accel;
	uint8_t		buttons;
}	t_mouse;

void		accel_init(t_accel *a);
int			accel_set(t_accel *a, uint32_t speed, uint32_t enabled);
uint32_t	accel_gain(const t_accel *a, int32_t dx, int32_t dy);
void		accel_apply(t_accel *a, int32_t *dx, int32_t *dy);
void		mouse_init(t_mouse *m);
int			mouse_events(t_mouse *m, const t_mousepkt *p, uint32_t mods,
				t_inpevent *out);

#endif
