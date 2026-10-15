#ifndef LAYOUT_H
# define LAYOUT_H

# include <stdint.h>

# define K_DEAD 0x80000000u
# define DEAD_CIRC 0x8000005eu
# define DEAD_TREMA 0x800000a8u
# define DEAD_TILDE 0x8000007eu
# define DEAD_GRAVE 0x80000060u
# define KF_LETTER 0x01
# define KF_CAPSCH 0x02
# define KF_PAD 0x04
# define KF_FIXED 0x08
# define KVK_CLEAR 0x0c
# define COL_NORMAL 0
# define COL_SHIFT 1
# define COL_ALTGR 2
# define COL_SHIFT_ALTGR 3
# define COL_CAPS 4

typedef struct s_key
{
	uint16_t	code;
	uint8_t		vk;
	uint8_t		vk_num;
	uint8_t		flags;
	uint32_t	ch[5];
}	t_key;

typedef struct s_layout
{
	const char	*name;
	const t_key	*keys;
	uint32_t	nkeys;
	uint32_t	altgr;
}	t_layout;

typedef struct s_compose
{
	uint32_t	accent;
	uint32_t	base;
	uint32_t	result;
}	t_compose;

extern const t_layout	g_layout_us;
extern const t_layout	g_layout_fr;

const t_layout	*layout_find(const char *name);
const t_layout	*layout_default(void);
const t_key		*layout_key(const t_layout *l, uint16_t code);
const t_key		*common_key(uint16_t code);
uint32_t		dead_compose(uint32_t accent, uint32_t base);

#endif
