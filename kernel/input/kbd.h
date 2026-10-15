#ifndef KBD_H
# define KBD_H

# include <stdint.h>
# include "inp_types.h"
# include "layout.h"
# include "velum/abi/abi_input.h"

# define KM_LSHIFT 0x01
# define KM_RSHIFT 0x02
# define KM_LCTRL 0x04
# define KM_RCTRL 0x08
# define KM_LALT 0x10
# define KM_RALT 0x20
# define KM_LWIN 0x40
# define KM_RWIN 0x80
# define KM_SHIFT 0x03
# define KM_CTRL 0x0c
# define KM_ALT 0x30
# define KM_WIN 0xc0
# define KBD_HELD_MAX 8

typedef struct s_kbd
{
	const t_layout	*layout;
	uint8_t			held;
	uint8_t			locks;
	uint32_t		dead;
}	t_kbd;

typedef struct s_modmap
{
	uint16_t	code;
	uint8_t		held;
	uint8_t		lock;
}	t_modmap;

void		kbd_init(t_kbd *k);
int			kbd_translate(t_kbd *k, const t_keyraw *raw, t_inpevent *out);
uint32_t	kbd_mods(const t_kbd *k);
uint32_t	kbd_resolve(const t_kbd *k, const t_key *key);
int			kbd_mod_apply(t_kbd *k, const t_keyraw *raw);
int			kbd_altgr(const t_kbd *k);
int			kbd_numpad(const t_kbd *k);
int			dead_apply(t_kbd *k, uint32_t ch, uint32_t *out);
int			kbd_release_held(t_kbd *k, t_inpevent *out);
int			kbd_ghost_release(const t_kbd *k, const t_keyraw *raw);

#endif
