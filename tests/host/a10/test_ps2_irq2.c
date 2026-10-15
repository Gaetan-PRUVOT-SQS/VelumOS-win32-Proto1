#include "harness.h"
#include "th.h"

static void	modifiers_through_irq(void)
{
	static const uint8_t	seq[] = {0x12, 0x15, 0xf0, 0x15, 0xf0, 0x12};
	t_inpevent				ev[8];

	th_boot_ready();
	h_eq_i64("Maj, a, relachements", th_kbd_in(seq, 6, ev, 8), 5);
	h_true(th_ev_is(&ev[2], INP_CHAR, 'A'), "A majuscule");
	h_eq_u64("mods : Maj + Verr Num", ev[2].mods, INPM_SHIFT | INPM_NUM);
	h_eq_u64("Maj relachee", ev[4].mods & INPM_SHIFT, 0);
}

static void	error_bytes_reset_decoders(void)
{
	static const uint8_t	pkt[] = {0x09, 0x01, 0x02, 0x00};
	t_inpevent				ev[8];

	th_boot_ready();
	f8042_push(0xe0, 0);
	f8042_push_err(0x00, 0);
	f8042_push(0x75, 0);
	f8042_push(0x75, 0);
	th_irq();
	h_eq_i64("E0, erreur, 75 jete, 75", th_pop_all(ev, 8), 2);
	h_true(th_ev_is(&ev[0], INP_KEY_DOWN, VK_NUMPAD0 + 8), "75 seul = pave 8");
	h_eq_u64("pas de drapeau etendu", ev[0].mods & INPM_EXTENDED, 0);
	f8042_push(0x09, 1);
	f8042_push_err(0x55, 1);
	h_eq_i64("paquet souris coupe par une erreur", th_mouse_in(pkt, 4, ev, 8),
		2);
	h_eq_i64("paquet suivant intact : x", ev[0].x, 1);
}

static void	drain_is_bounded(void)
{
	t_inpevent	ev[8];
	int			i;

	th_boot_ready();
	i = 0;
	while (i < 60)
	{
		f8042_push(0x00, 0);
		i++;
	}
	th_irq();
	h_eq_i64("32 octets au plus par IRQ", f8042_pending(), 28);
	th_irq();
	h_eq_i64("tout est consomme", f8042_pending(), 0);
	h_eq_i64("octets 0x00 ignores", th_pop_all(ev, 8), 0);
	th_irq();
	h_eq_i64("IRQ parasite sans effet", th_pop_all(ev, 8), 0);
	h_eq_i64("verrous equilibres", g_fsync.held + g_fsync.errors, 0);
}

static void	disabled_mouse_ignored(void)
{
	static const uint8_t	pkt[] = {0x09, 0x05, 0x03, 0x00};
	t_inpevent				ev[8];

	th_ps2_reset();
	g_f8042.mouse_present = 0;
	ps2_boot();
	th_irq();
	input_pop(&ev[0]);
	h_eq_i64("octets souris sans souris : ignores", th_mouse_in(pkt, 4, ev, 8),
		0);
	h_eq_i64("octets consommes", f8042_pending(), 0);
}

int	main(void)
{
	h_begin("a10/ps2_irq2");
	h_run("modificateurs", modifiers_through_irq);
	h_run("erreurs de parite", error_bytes_reset_decoders);
	h_run("vidage borne", drain_is_bounded);
	h_run("souris absente", disabled_mouse_ignored);
	return (h_end());
}
