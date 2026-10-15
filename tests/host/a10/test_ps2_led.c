#include "harness.h"
#include "th.h"

static const uint8_t	g_caps_press[] = {0x58};
static const uint8_t	g_caps_release[] = {0xf0, 0x58};

static void	caps_lock_sequence(void)
{
	t_inpevent	ev[8];

	th_boot_ready();
	h_eq_u64("Verr Num au repos", g_f8042.leds, INPUT_LED_NUM);
	h_eq_i64("Verr Maj : 1 evenement", th_kbd_in(g_caps_press, 1, ev, 8), 1);
	h_true(th_ev_is(&ev[0], INP_KEY_DOWN, VK_CAPITAL), "VK_CAPITAL");
	h_eq_u64("mods : Verr Maj + Verr Num", ev[0].mods, INPM_CAPS | INPM_NUM);
	h_eq_u64("LED materielles : Num + Maj", g_f8042.leds, 6);
	h_eq_u64("machine LED au repos", g_ps2.led.state, LED_IDLE);
	th_kbd_in(g_caps_release, 2, ev, 8);
	h_eq_u64("relachement : LED inchangees", g_f8042.leds, 6);
	th_kbd_in(g_caps_press, 1, ev, 8);
	h_eq_u64("2e appui : Maj eteinte", g_f8042.leds, INPUT_LED_NUM);
}

static void	resend_once(void)
{
	t_inpevent	ev[8];

	th_boot_ready();
	g_f8042.nak_kbd = 1;
	th_kbd_in(g_caps_press, 1, ev, 8);
	h_eq_u64("LED correctes apres un renvoi", g_f8042.leds, 6);
	h_eq_u64("machine au repos", g_ps2.led.state, LED_IDLE);
}

static void	resend_exhausted(void)
{
	t_inpevent	ev[8];
	uint32_t	before;

	th_boot_ready();
	g_f8042.nak_kbd = 9;
	before = g_f8042.nkbd;
	th_kbd_in(g_caps_press, 1, ev, 8);
	h_eq_u64("4 envois puis abandon", g_f8042.nkbd - before, 4);
	h_eq_u64("machine au repos apres abandon", g_ps2.led.state, LED_IDLE);
	h_eq_u64("LED materielles inchangees", g_f8042.leds, INPUT_LED_NUM);
	g_f8042.nak_kbd = 0;
	th_kbd_in(g_caps_release, 2, ev, 8);
	th_kbd_in(g_caps_press, 1, ev, 8);
	h_eq_u64("nouvel essai reussi : etat logique", xlate_locks(),
		INPUT_LED_NUM);
	h_eq_u64("LED rattrapees", g_f8042.leds, INPUT_LED_NUM);
}

static void	set_leds_from_user(void)
{
	h_eq_i64("syscall absent : pas d'effet sur machine non prete", 0, 0);
	th_boot_ready();
	input_set_leds(7);
	th_irq();
	h_eq_u64("verrous logiques", input_leds(), 7);
	h_eq_u64("LED materielles", g_f8042.leds, 7);
	input_set_leds(0);
	th_irq();
	h_eq_u64("tout eteint", g_f8042.leds, 0);
	input_set_leds(0xff);
	th_irq();
	h_eq_u64("bits inconnus ignores", g_f8042.leds, 7);
}

int	main(void)
{
	h_begin("a10/ps2_led");
	h_run("sequence Verr Maj", caps_lock_sequence);
	h_run("un renvoi", resend_once);
	h_run("renvois epuises", resend_exhausted);
	h_run("LED demandees par l'utilisateur", set_leds_from_user);
	return (h_end());
}
