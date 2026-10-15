#include "harness.h"
#include "th.h"

static void	all_eight_released(void)
{
	static const uint8_t	down[] = {0x12, 0x59, 0x14, 0xe0, 0x14, 0x11, 0xe0,
		0x11, 0xe0, 0x1f, 0xe0, 0x27};
	static const uint8_t	lost[] = {0xff};
	t_inpevent				ev[16];
	int						i;

	th_boot_ready();
	memset(ev, 0, sizeof(ev));
	h_eq_i64("8 modificateurs enfonces", th_kbd_in(down, 12, ev, 16), 8);
	h_eq_u64("etat enfonce", g_xlate.kbd.held, 0xff);
	h_eq_i64("0xFF : 8 relachements", th_kbd_in(lost, 1, ev, 16), 8);
	i = 0;
	while (i < 8)
	{
		h_eq_u64("type KEY_UP", ev[i].type, INP_KEY_UP);
		i++;
	}
	h_eq_u64("aucun modificateur dans le dernier", ev[7].mods & (INPM_SHIFT
			| INPM_CTRL | INPM_ALT | INPM_WIN), 0);
	h_eq_u64("etat remis a zero", g_xlate.kbd.held, 0);
}

static void	nothing_held_nothing_sent(void)
{
	static const uint8_t	lost[] = {0x00, 0xff, 0xaa};
	static const uint8_t	caps[] = {0x58, 0xf0, 0x58};
	t_inpevent				ev[8];

	th_boot_ready();
	h_eq_i64("Verr Maj", th_kbd_in(caps, 3, ev, 8), 2);
	h_eq_i64("perte sans modificateur : rien", th_kbd_in(lost, 3, ev, 8), 0);
	h_eq_u64("verrous conserves", g_xlate.kbd.locks, INPUT_LED_NUM
		| INPUT_LED_CAPS);
	h_eq_u64("verrous equilibres", g_fsync.held + g_fsync.errors, 0);
}

static void	ack_is_not_a_loss(void)
{
	static const uint8_t	down[] = {0x12};
	static const uint8_t	ack[] = {0xfa, 0xfe};
	static const uint8_t	up[] = {0xf0, 0x12};
	t_inpevent				ev[8];

	th_boot_ready();
	memset(ev, 0, sizeof(ev));
	h_eq_i64("Maj enfoncee", th_kbd_in(down, 1, ev, 8), 1);
	h_eq_i64("0xFA et 0xFE : aucun evenement", th_kbd_in(ack, 2, ev, 8), 0);
	h_eq_u64("Maj toujours enfoncee", g_xlate.kbd.held, KM_LSHIFT);
	h_eq_i64("relachement normal", th_kbd_in(up, 2, ev, 8), 1);
	h_true(th_ev_is(&ev[0], INP_KEY_UP, VK_SHIFT), "KEY_UP Maj");
	h_eq_u64("etat relache", g_xlate.kbd.held, 0);
}

int	main(void)
{
	h_begin("a10/kbd_lost2");
	h_run("8 modificateurs, 0xFF, 8 relachements", all_eight_released);
	h_run("perte sans modificateur, verrous conserves",
		nothing_held_nothing_sent);
	h_run("0xFA et 0xFE ne sont pas des signaux de perte", ack_is_not_a_loss);
	return (h_end());
}
