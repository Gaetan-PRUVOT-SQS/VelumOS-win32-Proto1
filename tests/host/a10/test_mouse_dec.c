#include "harness.h"
#include "th.h"

static const t_mdcase	g_cases[] = {
{"immobile", {0x08, 0, 0}, 0, 0, 0, 0, 0},
{"bouton gauche", {0x09, 5, 3}, 0, 5, 3, 0, 1},
{"bouton droit", {0x0a, 1, 1}, 0, 1, 1, 0, 2},
{"bouton milieu", {0x0c, 1, 1}, 0, 1, 1, 0, 4},
{"trois boutons", {0x0f, 0, 0}, 0, 0, 0, 0, 7},
{"negatifs -1 -1", {0x38, 0xff, 0xff}, 0, -1, -1, 0, 0},
{"limite +255", {0x08, 0xff, 0}, 0, 255, 0, 0, 0},
{"limite -256", {0x18, 0x00, 0}, 0, -256, 0, 0, 0},
{"limite y +255", {0x08, 0, 0xff}, 0, 0, 255, 0, 0},
{"limite y -256", {0x28, 0, 0x00}, 0, 0, -256, 0, 0},
{"debordement x ignore", {0x48, 0x7f, 0x01}, 0, 0, 1, 0, 0},
{"debordement y ignore", {0x88, 0x01, 0x7f}, 0, 1, 0, 0, 0},
{"debordements x et y", {0xc8, 0x7f, 0x7f}, 0, 0, 0, 0, 0},
{"debordement garde les boutons", {0xc9, 1, 1}, 0, 0, 0, 0, 1},
{"molette +1", {0x08, 0, 0, 0x01}, 1, 0, 0, 1, 0},
{"molette -1", {0x08, 0, 0, 0xff}, 1, 0, 0, -1, 0},
{"molette +127", {0x08, 0, 0, 0x7f}, 1, 0, 0, 127, 0},
{"molette -128", {0x08, 0, 0, 0x80}, 1, 0, 0, -128, 0},
{"molette et mouvement", {0x09, 2, 3, 0xfe}, 1, 2, 3, -2, 1},
};

static void	run_table(void)
{
	uint32_t	i;

	i = 0;
	while (i < sizeof(g_cases) / sizeof(g_cases[0]))
	{
		th_run_mdcase(&g_cases[i]);
		i++;
	}
}

static void	resync_on_sync_bit(void)
{
	static const uint8_t	stream[] = {0x01, 0x02, 0x03, 0x09, 0x10, 0x20,
		0x00, 0x0a, 0x30, 0x40};
	t_mousedec				d;
	t_mousepkt				pkt;
	int						packets;
	uint32_t				i;

	mousedec_reset(&d, 0);
	packets = 0;
	i = 0;
	while (i < sizeof(stream))
	{
		if (mousedec_feed(&d, stream[i], 100 + i, &pkt))
		{
			packets++;
			h_eq_i64("paquet valide : dx", pkt.dx, 0x10 + 0x20 * (packets - 1));
		}
		i++;
	}
	h_eq_i64("deux paquets apres octets parasites", packets, 2);
}

static void	lost_byte_recovers(void)
{
	static const uint8_t	stream[] = {0x09, 0x20, 0x09, 0x30, 0x40, 0x0a,
		0x11, 0x22, 0x09, 0x33, 0x44};
	t_mousedec				d;
	t_mousepkt				pkt;
	int						good;
	uint32_t				i;

	mousedec_reset(&d, 0);
	good = 0;
	i = 0;
	while (i < sizeof(stream))
	{
		if (mousedec_feed(&d, stream[i], 100 + i, &pkt) && pkt.dx >= 0x11)
			good++;
		i++;
	}
	h_true(good >= 2, "les paquets suivants une perte sont decodes");
	h_eq_u64("aucun octet en attente", d.len, 0);
}

static void	stalled_packet_dropped(void)
{
	t_mousedec	d;
	t_mousepkt	pkt;

	mousedec_reset(&d, 0);
	mousedec_feed(&d, 0x09, 1000, &pkt);
	mousedec_feed(&d, 0x01, 2000, &pkt);
	h_eq_i64("paquet frais apres 51 ms", mousedec_feed(&d, 0x0a, 51002000,
			&pkt), 0);
	mousedec_feed(&d, 0x05, 51003000, &pkt);
	h_eq_i64("paquet complet", mousedec_feed(&d, 0x06, 51004000, &pkt), 1);
	h_eq_i64("dx du paquet frais", pkt.dx, 5);
	mousedec_reset(&d, 0);
	mousedec_feed(&d, 0x09, 1000, &pkt);
	mousedec_feed(&d, 0x01, 2000, &pkt);
	h_eq_i64("exactement 50 ms : conserve", mousedec_feed(&d, 0x02,
			50002000, &pkt), 1);
}

int	main(void)
{
	h_begin("a10/mouse_dec");
	h_run("paquets valides et limites", run_table);
	h_run("resynchronisation par le bit 3", resync_on_sync_bit);
	h_run("octet perdu", lost_byte_recovers);
	h_run("paquet interrompu", stalled_packet_dropped);
	return (h_end());
}
