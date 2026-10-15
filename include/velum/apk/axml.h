#ifndef AXML_H
# define AXML_H

# include "velum/apk/respool.h"

# define AXML_DONE 0
# define AXML_START 1
# define AXML_END 2
# define AXML_TEXT 3
# define AXML_MAX_DEPTH 64
# define AXML_NO_STRING 0xffffffff

# define RES_T_NULL 0x00
# define RES_T_REFERENCE 0x01
# define RES_T_ATTRIBUTE 0x02
# define RES_T_STRING 0x03
# define RES_T_FLOAT 0x04
# define RES_T_DIMENSION 0x05
# define RES_T_FRACTION 0x06
# define RES_T_INT_DEC 0x10
# define RES_T_INT_HEX 0x11
# define RES_T_INT_BOOLEAN 0x12
# define RES_T_COLOR_ARGB8 0x1c
# define RES_T_COLOR_RGB8 0x1d
# define RES_T_COLOR_ARGB4 0x1e
# define RES_T_COLOR_RGB4 0x1f

# define AXML_ATTR_LABEL 0x01010001
# define AXML_ATTR_ICON 0x01010002
# define AXML_ATTR_NAME 0x01010003
# define AXML_ATTR_DEBUGGABLE 0x0101000f
# define AXML_ATTR_ORIENTATION 0x010100c4
# define AXML_ATTR_ID 0x010100d0
# define AXML_ATTR_LAYOUT_WIDTH 0x010100f4
# define AXML_ATTR_LAYOUT_HEIGHT 0x010100f5
# define AXML_ATTR_TEXT 0x0101014f
# define AXML_ATTR_MIN_SDK 0x0101020c
# define AXML_ATTR_VERSION_CODE 0x0101021b
# define AXML_ATTR_VERSION_NAME 0x0101021c
# define AXML_ATTR_TARGET_SDK 0x01010270

typedef struct s_axmlattr
{
	uint32_t	name;
	uint32_t	res_id;
	uint32_t	raw;
	uint32_t	type;
	uint32_t	data;
	uint32_t	ns;
}	t_axmlattr;

typedef struct s_axml
{
	t_span		file;
	t_respool	pool;
	uint32_t	map_off;
	uint32_t	map_count;
	uint32_t	pos;
	uint32_t	depth;
	uint32_t	ev;
	uint32_t	ev_off;
	uint32_t	attr_off;
	uint32_t	attr_size;
	uint32_t	attr_count;
}	t_axml;

int			axml_open(t_axml *x, t_span file);
int			axml_next(t_axml *x);
int			axml_name(const t_axml *x, t_text out);
uint32_t	axml_attr_count(const t_axml *x);
int			axml_attr(const t_axml *x, uint32_t i, t_axmlattr *out);
int			axml_attr_find(const t_axml *x, uint32_t res_id, t_axmlattr *out);
int			axml_string(const t_axml *x, uint32_t index, t_text out);
int			axml_text(const t_axml *x, t_text out);

#endif
