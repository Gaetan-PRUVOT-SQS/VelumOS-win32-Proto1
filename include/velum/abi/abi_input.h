#ifndef ABI_INPUT_H
# define ABI_INPUT_H

# include <stdint.h>

# define INP_KEY_DOWN 1
# define INP_KEY_UP 2
# define INP_CHAR 3
# define INP_MOUSE_MOVE 4
# define INP_MOUSE_DOWN 5
# define INP_MOUSE_UP 6
# define INP_WHEEL 7

# define INPM_SHIFT 0x01
# define INPM_CTRL 0x02
# define INPM_ALT 0x04
# define INPM_WIN 0x08
# define INPM_CAPS 0x10
# define INPM_NUM 0x20
# define INPM_REPEAT 0x40
# define INPM_EXTENDED 0x80

# define BTN_LEFT 0
# define BTN_RIGHT 1
# define BTN_MIDDLE 2

# define VK_BACK 0x08
# define VK_TAB 0x09
# define VK_RETURN 0x0d
# define VK_SHIFT 0x10
# define VK_CONTROL 0x11
# define VK_MENU 0x12
# define VK_PAUSE 0x13
# define VK_CAPITAL 0x14
# define VK_ESCAPE 0x1b
# define VK_SPACE 0x20
# define VK_PRIOR 0x21
# define VK_NEXT 0x22
# define VK_END 0x23
# define VK_HOME 0x24
# define VK_LEFT 0x25
# define VK_UP 0x26
# define VK_RIGHT 0x27
# define VK_DOWN 0x28
# define VK_SNAPSHOT 0x2c
# define VK_INSERT 0x2d
# define VK_DELETE 0x2e
# define VK_LWIN 0x5b
# define VK_RWIN 0x5c
# define VK_APPS 0x5d
# define VK_NUMPAD0 0x60
# define VK_MULTIPLY 0x6a
# define VK_ADD 0x6b
# define VK_SUBTRACT 0x6d
# define VK_DECIMAL 0x6e
# define VK_DIVIDE 0x6f
# define VK_F1 0x70
# define VK_F12 0x7b
# define VK_NUMLOCK 0x90
# define VK_SCROLL 0x91
# define VK_LSHIFT 0xa0
# define VK_RSHIFT 0xa1
# define VK_LCONTROL 0xa2
# define VK_RCONTROL 0xa3
# define VK_LMENU 0xa4
# define VK_RMENU 0xa5
# define VK_OEM_1 0xba
# define VK_OEM_PLUS 0xbb
# define VK_OEM_COMMA 0xbc
# define VK_OEM_MINUS 0xbd
# define VK_OEM_PERIOD 0xbe
# define VK_OEM_2 0xbf
# define VK_OEM_3 0xc0
# define VK_OEM_4 0xdb
# define VK_OEM_5 0xdc
# define VK_OEM_6 0xdd
# define VK_OEM_7 0xde
# define VK_OEM_8 0xdf
# define VK_OEM_102 0xe2

typedef struct s_inpevent
{
	uint32_t	type;
	uint32_t	code;
	int32_t		x;
	int32_t		y;
	uint32_t	mods;
	uint32_t	scancode;
	uint64_t	time_ns;
}	t_inpevent;

# define INPUT_KIND_ALL 0
# define INPUT_KIND_KEYBOARD 1
# define INPUT_KIND_MOUSE 2
# define INPUT_LAYOUT_GET 0
# define INPUT_LAYOUT_SET 1
# define INPUT_LAYOUT_NAME_MAX 15
# define INPUT_LED_SCROLL 0x1
# define INPUT_LED_NUM 0x2
# define INPUT_LED_CAPS 0x4
# define INPUT_LED_ALL 0x7
# define INPUT_MOUSE_SPEED_MIN 1
# define INPUT_MOUSE_SPEED_MAX 20
# define INPUT_QUEUE_LEN 256

#endif
