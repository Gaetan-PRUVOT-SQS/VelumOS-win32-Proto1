#include "harness.h"
#include "th.h"

static void	lock_keys_toggle(void)
{
	static const uint16_t	code[3] = {0x58, 0x77, 0x7e};
	static const uint8_t	bit[3] = {INPUT_LED_CAPS, INPUT_LED_NUM,
		INPUT_LED_SCROLL};
	t_kbd					k;
	t_inpevent				ev[KEY_OUT_MAX];
	int						i;

	i = 0;
	while (i < 3)
	{
		kbd_init(&k);
		k.locks = 0;
		th_press(&k, code[i], ev);
		h_eq_u64("verrou allume a l'appui", k.locks, bit[i]);
		th_release(&k, code[i], ev);
		h_eq_u64("verrou garde au relachement", k.locks, bit[i]);
		kbd_translate(&k, &(t_keyraw){code[i], 0, 1}, ev);
		h_eq_u64("repetition sans bascule", k.locks, bit[i]);
		th_press(&k, code[i], ev);
		h_eq_u64("verrou eteint au 2e appui", k.locks, 0);
		i++;
	}
}

static void	modifier_state(void)
{
	t_kbd		k;
	t_inpevent	ev[KEY_OUT_MAX];

	kbd_init(&k);
	th_press(&k, 0x12, ev);
	th_press(&k, 0x59, ev);
	th_release(&k, 0x12, ev);
	h_eq_u64("Maj droite encore active", kbd_mods(&k) & INPM_SHIFT, INPM_SHIFT);
	th_release(&k, 0x59, ev);
	h_eq_u64("plus de Maj", kbd_mods(&k) & INPM_SHIFT, 0);
	h_eq_u64("evenement KEY_UP : Maj retiree", ev[0].mods & INPM_SHIFT, 0);
	th_press(&k, 0xe014, ev);
	th_press(&k, 0x14, ev);
	th_release(&k, 0x14, ev);
	h_eq_u64("Ctrl droit actif", kbd_mods(&k) & INPM_CTRL, INPM_CTRL);
	th_release(&k, 0xe014, ev);
	h_eq_u64("held vide", k.held, 0);
}

static void	key_up_and_repeat(void)
{
	t_kbd		k;
	t_inpevent	ev[KEY_OUT_MAX];
	t_keyraw	rep;

	kbd_init(&k);
	th_press(&k, 0x15, ev);
	h_eq_i64("relachement : un evenement", th_release(&k, 0x15, ev), 1);
	h_eq_u64("type KEY_UP", ev[0].type, INP_KEY_UP);
	h_eq_u64("code KEY_UP", ev[0].code, 'A');
	rep = th_raw(0x15, 0, 1);
	h_eq_i64("repetition : deux evenements", kbd_translate(&k, &rep, ev), 2);
	h_eq_u64("REPEAT sur KEY_DOWN", ev[0].mods & INPM_REPEAT, INPM_REPEAT);
	h_eq_u64("REPEAT sur CHAR", ev[1].mods & INPM_REPEAT, INPM_REPEAT);
	th_press(&k, 0x15, ev);
	h_eq_u64("pas de REPEAT hors repetition", ev[0].mods & INPM_REPEAT, 0);
}

static void	layout_change(void)
{
	t_kbd		k;

	kbd_init(&k);
	h_eq_u64("fr : touche q = a", th_char(&k, 0x15), 'a');
	k.layout = &g_layout_us;
	h_eq_u64("us : touche q = q", th_char(&k, 0x15), 'q');
	h_eq_u64("us : touche 0x4c = ;", th_char(&k, 0x4c), ';');
	k.layout = &g_layout_fr;
	h_eq_u64("fr : touche 0x4c = m", th_char(&k, 0x4c), 'm');
	h_eq_u64("verrou Num par defaut", k.locks, INPUT_LED_NUM);
}

int	main(void)
{
	h_begin("a10/kbd_state");
	h_run("verrous : bascule", lock_keys_toggle);
	h_run("modificateurs gauche/droite", modifier_state);
	h_run("relachement et repetition", key_up_and_repeat);
	h_run("changement de disposition", layout_change);
	return (h_end());
}
