#include "ctl_int.h"

int32_t	ctl_wheel_delta(int32_t wheel)
{
	if (wheel > 1024)
		wheel = 1024;
	if (wheel < -1024)
		wheel = -1024;
	return (wheel * CTL_WHEEL_ROWS);
}
