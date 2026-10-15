#include <stdio.h>
#include "harness.h"
#include "th.h"

static void	kcase_check_char(const t_kcase *c, const t_inpevent *ev)
{
	if (c->ch == 0)
		return ;
	h_true(th_ev_is(&ev[1], INP_CHAR, c->ch), "evenement CHAR");
	h_eq_u64("mods du CHAR", ev[1].mods, c->chmods);
	h_eq_u64("scancode du CHAR", ev[1].scancode, c->key);
}

void	th_run_kcase(const t_layout *l, const t_kcase *c)
{
	t_kbd		k;
	t_inpevent	ev[KEY_OUT_MAX];
	int			i;
	int			n;

	g_h.name = c->name;
	kbd_init(&k);
	k.layout = l;
	i = 0;
	while (i < 3 && c->pre[i])
		th_press(&k, c->pre[i++], ev);
	k.locks = c->locks;
	n = th_press(&k, c->key, ev);
	h_eq_i64("nombre d'evenements", n, c->n);
	if (n < 1)
		return ;
	h_eq_u64("type KEY_DOWN", ev[0].type, INP_KEY_DOWN);
	h_eq_u64("code virtuel", ev[0].code, c->vk);
	h_eq_u64("mods du KEY_DOWN", ev[0].mods, c->mods);
	h_eq_u64("scancode", ev[0].scancode, c->key);
	if (n >= 2)
		kcase_check_char(c, ev);
}

int	th_dead_step(t_kbd *k, uint32_t step, uint32_t *out)
{
	t_inpevent	ev[KEY_OUT_MAX];
	int			n;
	int			i;
	int			chars;

	if (step & TH_SH)
		th_press(k, 0x12, ev);
	if (step & TH_AG)
		th_press(k, 0xe011, ev);
	n = th_press(k, (uint16_t)(step & 0xffff), ev);
	chars = 0;
	i = 1;
	while (i < n)
		out[chars++] = ev[i++].code;
	th_release(k, (uint16_t)(step & 0xffff), ev);
	if (step & TH_AG)
		th_release(k, 0xe011, ev);
	if (step & TH_SH)
		th_release(k, 0x12, ev);
	return (chars);
}

void	th_run_dead(const t_deadcase *c)
{
	t_kbd		k;
	uint32_t	got[16];
	int			n;
	int			i;

	g_h.name = c->name;
	kbd_init(&k);
	n = 0;
	i = 0;
	while (i < 4 && c->step[i])
		n += th_dead_step(&k, c->step[i++], &got[n]);
	i = 0;
	while (i < n && i < 3 && got[i] == c->out[i])
		i++;
	h_eq_i64("caracteres produits", n, (c->out[0] != 0) + (c->out[1] != 0)
		+ (c->out[2] != 0));
	h_eq_i64("caracteres conformes", i, n);
}

void	th_run_mdcase(const t_mdcase *c)
{
	t_mousedec	d;
	t_mousepkt	pkt;
	int			i;
	int			got;

	g_h.name = c->name;
	mousedec_reset(&d, c->wheel);
	got = 0;
	i = 0;
	while (i < 3 + c->wheel)
	{
		got += mousedec_feed(&d, c->in[i], 1000 + (uint64_t)i, &pkt);
		i++;
	}
	h_eq_i64("un paquet", got, 1);
	h_eq_i64("dx", pkt.dx, c->dx);
	h_eq_i64("dy", pkt.dy, c->dy);
	h_eq_i64("dz", pkt.dz, c->dz);
	h_eq_u64("boutons", pkt.buttons, c->btn);
	h_eq_u64("tampon vide", d.len, 0);
}
