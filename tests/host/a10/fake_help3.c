#include <stdio.h>
#include "harness.h"
#include "th.h"

void	th_check_col(const t_layout *l, const uint16_t *pos, const t_colcase *c)
{
	t_kbd		k;
	uint32_t	got;
	int			i;

	kbd_init(&k);
	k.layout = l;
	k.held = c->held;
	k.locks = c->locks;
	g_h.name = c->name;
	i = 0;
	while (i < TH_KEYS)
	{
		got = th_resolve(&k, pos[i]);
		if (got != c->exp[i])
			fprintf(stderr, "ECHEC %s/%s : touche 0x%02x obtenu 0x%x attendu "
				"0x%x\n", g_h.suite, c->name, pos[i], got, c->exp[i]);
		h_true(got == c->exp[i], "caractere de la touche");
		i++;
	}
}

void	th_check_vk(const t_layout *l, const uint16_t *pos, const uint8_t *vk)
{
	const t_key	*key;
	int			i;

	i = 0;
	while (i < TH_KEYS)
	{
		key = layout_key(l, pos[i]);
		h_true(key != NULL, "touche presente");
		if (key)
			h_eq_u64("code virtuel", key->vk, vk[i]);
		i++;
	}
}

uint32_t	th_resolve(t_kbd *k, uint16_t code)
{
	const t_key	*key;

	key = layout_key(k->layout, code);
	if (!key)
		return (0xdeadbeef);
	return (kbd_resolve(k, key));
}

int	th_layout_dups(const t_layout *l)
{
	uint32_t	i;
	uint32_t	j;
	int			dup;

	dup = 0;
	i = 0;
	while (i < l->nkeys)
	{
		j = i + 1;
		while (j < l->nkeys)
		{
			dup += (l->keys[i].code == l->keys[j].code);
			j++;
		}
		i++;
	}
	return (dup);
}

int	th_layout_collisions(const t_layout *l)
{
	uint32_t	i;
	int			hits;

	hits = 0;
	i = 0;
	while (i < l->nkeys)
	{
		hits += (common_key(l->keys[i].code) != NULL);
		i++;
	}
	return (hits);
}
