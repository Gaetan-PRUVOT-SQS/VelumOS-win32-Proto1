#include "harness.h"
#include "th.h"

static const uint8_t	g_caps[] = {0x58};
static const uint8_t	g_ack[] = {0xfa};

static void	silent_keyboard_times_out(void)
{
	static const uint8_t	key[] = {0x1c};
	t_inpevent				ev[8];

	th_boot_ready();
	g_f8042.kbd_silent = 1;
	th_kbd_in(g_caps, 1, ev, 8);
	h_eq_u64("attente de l'ACK de commande", g_ps2.led.state, LED_ACK_CMD);
	g_ftime.now += 600000000ull;
	h_eq_i64("touche apres 600 ms : decodee", th_kbd_in(key, 1, ev, 8), 2);
	h_eq_u64("machine rendue au repos", g_ps2.led.state, LED_IDLE);
	g_f8042.kbd_silent = 0;
	th_kbd_in(g_caps, 1, ev, 8);
	h_eq_u64("LED rattrapees ensuite", g_f8042.leds, INPUT_LED_NUM);
}

static void	scancode_between_acks(void)
{
	static const uint8_t	mix[] = {0x1c, 0xfa};
	t_inpevent				ev[8];

	th_boot_ready();
	g_f8042.kbd_silent = 1;
	th_kbd_in(g_caps, 1, ev, 8);
	h_eq_i64("touche entre deux ACK : decodee", th_kbd_in(mix, 2, ev, 8), 2);
	h_eq_u64("ACK consomme : octet de donnee envoye", g_ps2.led.state,
		LED_ACK_ARG);
	h_eq_u64("octet de donnee = verrous", g_f8042.kbd_log[g_f8042.nkbd - 1],
		6);
	th_kbd_in(g_ack, 1, ev, 8);
	h_eq_u64("sequence terminee", g_ps2.led.state, LED_IDLE);
}

static void	state_changes_during_sequence(void)
{
	t_inpevent	ev[8];

	th_boot_ready();
	g_f8042.kbd_silent = 1;
	th_kbd_in(g_caps, 1, ev, 8);
	th_kbd_in(g_ack, 1, ev, 8);
	h_eq_u64("donnee envoyee", g_ps2.led.sent, 6);
	th_kbd_in((const uint8_t[]){0xf0, 0x58}, 2, ev, 8);
	th_kbd_in(g_caps, 1, ev, 8);
	h_eq_u64("changement pendant la sequence : voulu", g_ps2.led.want, 2);
	g_f8042.kbd_silent = 0;
	th_kbd_in(g_ack, 1, ev, 8);
	h_eq_u64("nouvelle sequence terminee", g_ps2.led.state, LED_IDLE);
	h_eq_u64("LED finales = etat logique", g_f8042.leds, INPUT_LED_NUM);
}

int	main(void)
{
	h_begin("a10/ps2_led2");
	h_run("clavier muet : delai de la machine", silent_keyboard_times_out);
	h_run("octet de touche entre deux ACK", scancode_between_acks);
	h_run("changement d'etat en cours de sequence",
		state_changes_during_sequence);
	return (h_end());
}
