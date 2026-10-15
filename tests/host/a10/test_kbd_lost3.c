#include "harness.h"
#include "th.h"

static void	caps_repeat_survives_loss(void)
{
	static const uint8_t	caps[] = {0x58};
	static const uint8_t	lost[] = {0x00};
	t_inpevent				ev[8];

	th_boot_ready();
	memset(ev, 0, sizeof(ev));
	h_eq_i64("Verr Maj enfoncee", th_kbd_in(caps, 1, ev, 8), 1);
	h_eq_u64("verrou bascule", g_xlate.kbd.locks, INPUT_LED_NUM
		| INPUT_LED_CAPS);
	h_eq_i64("repetition", th_kbd_in(caps, 1, ev, 8), 1);
	h_eq_i64("perte sans modificateur", th_kbd_in(lost, 1, ev, 8), 0);
	h_eq_i64("repetition apres la perte", th_kbd_in(caps, 1, ev, 8), 1);
	h_eq_u64("toujours marquee repetee", ev[0].mods & INPM_REPEAT,
		INPM_REPEAT);
	h_eq_u64("verrou bascule une seule fois", g_xlate.kbd.locks, INPUT_LED_NUM
		| INPUT_LED_CAPS);
}

static void	plain_repeat_survives_loss(void)
{
	static const uint8_t	key[] = {0x1c};
	static const uint8_t	lost[] = {0x00};
	t_inpevent				ev[8];

	th_boot_ready();
	memset(ev, 0, sizeof(ev));
	h_eq_i64("appui", th_kbd_in(key, 1, ev, 8), 2);
	h_eq_u64("premier appui non repete", ev[0].mods & INPM_REPEAT, 0);
	h_eq_i64("perte 0x00", th_kbd_in(lost, 1, ev, 8), 0);
	h_eq_i64("frappe apres la perte", th_kbd_in(key, 1, ev, 8), 2);
	h_eq_u64("marquee repetee apres 0x00", ev[0].mods & INPM_REPEAT,
		INPM_REPEAT);
	f8042_push_err(0xff, 0);
	th_irq();
	h_eq_i64("erreur de ligne sans modificateur", th_pop_all(ev, 8), 0);
	h_eq_i64("octet suivant jete", th_kbd_in(key, 1, ev, 8), 0);
	h_eq_i64("repetition apres l'erreur", th_kbd_in(key, 1, ev, 8), 2);
	h_eq_u64("toujours marquee repetee", ev[0].mods & INPM_REPEAT,
		INPM_REPEAT);
}

static void	dead_cleared_by_loss(void)
{
	static const uint8_t	circ[] = {0x54, 0xf0, 0x54};
	static const uint8_t	lost[] = {0x00};
	static const uint8_t	e[] = {0x24};
	t_inpevent				ev[8];

	th_boot_ready();
	memset(ev, 0, sizeof(ev));
	h_eq_i64("accent circonflexe mort", th_kbd_in(circ, 3, ev, 8), 2);
	h_true(g_xlate.kbd.dead != 0, "touche morte en attente");
	h_eq_i64("perte", th_kbd_in(lost, 1, ev, 8), 0);
	h_eq_u64("touche morte oubliee", g_xlate.kbd.dead, 0);
	h_eq_i64("lettre", th_kbd_in(e, 1, ev, 8), 2);
	h_true(th_ev_is(&ev[1], INP_CHAR, 'e'), "lettre nue");
}

int	main(void)
{
	h_begin("a10/kbd_lost3");
	h_run("Verr Maj repetee, perte 0x00, verrou bascule une fois",
		caps_repeat_survives_loss);
	h_run("touche tenue, perte 0x00 puis erreur, repetition conservee",
		plain_repeat_survives_loss);
	h_run("touche morte, perte, lettre nue", dead_cleared_by_loss);
	return (h_end());
}
