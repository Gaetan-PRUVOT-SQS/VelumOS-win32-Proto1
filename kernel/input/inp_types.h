#ifndef INP_TYPES_H
# define INP_TYPES_H

# include <stdint.h>

# define KEY_EXT_PREFIX 0xe000
# define KEY_PAUSE_CODE 0xe114
# define MOUSE_OUT_MAX 6
# define KEY_OUT_MAX 4

typedef struct s_keyraw
{
	uint16_t	code;
	uint8_t		release;
	uint8_t		repeat;
}	t_keyraw;

typedef struct s_mousepkt
{
	int32_t		dx;
	int32_t		dy;
	int32_t		dz;
	uint8_t		buttons;
}	t_mousepkt;

#endif
