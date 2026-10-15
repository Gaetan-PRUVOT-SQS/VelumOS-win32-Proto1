#include "sys_int.h"
#include "velum/vinput.h"

int64_t	v_input_open(uint32_t kind)
{
	return (sys1(SYS_INPUT_OPEN, kind));
}

int	v_input_read(t_handle input, t_inpevent *out, uint32_t max)
{
	return ((int)sys3(SYS_INPUT_READ, input, sys_ptr(out), max));
}

int	v_input_layout(uint32_t op, char *name, size_t name_len)
{
	return ((int)sys3(SYS_INPUT_LAYOUT, op, sys_ptr(name), name_len));
}

int	v_input_leds(uint32_t mask)
{
	return ((int)sys1(SYS_INPUT_LEDS, mask));
}

int	v_input_mouse_cfg(uint32_t speed, uint32_t accel)
{
	return ((int)sys2(SYS_INPUT_MOUSE_CFG, speed, accel));
}
