#ifndef A20_LOGON_CHECK_H
# define A20_LOGON_CHECK_H

# include <stdint.h>
# include "harness.h"
# include "layout.h"
# include "logon_layout.h"

static inline void	check_tiles(const t_logonlayout *o, t_rect screen)
{
	t_rect		local;
	uint32_t	i;
	uint32_t	j;

	local = lay_rect(0, 0, o->card.w, o->card.h);
	h_true(lay_inside(screen, o->card), "carte dans l'ecran");
	h_true(o->card.y >= LOGON_BAND_TOP, "carte sous le bandeau haut");
	h_true(o->card.y + o->card.h <= screen.h - LOGON_BAND_BOTTOM,
		"carte au dessus du bandeau bas");
	i = 0;
	while (i < o->count)
	{
		h_true(lay_inside(local, o->tile[i]), "tuile dans la carte");
		j = i + 1;
		while (j < o->count)
		{
			h_true(!lay_overlap(o->tile[i], o->tile[j]), "tuiles disjointes");
			j++;
		}
		h_true(!lay_overlap(o->tile[i], o->password), "tuile hors saisie");
		h_true(!lay_overlap(o->tile[i], o->shutdown), "tuile hors Eteindre");
		i++;
	}
}

static inline void	check_rest(const t_logonlayout *o, t_rect screen)
{
	t_rect	local;

	local = lay_rect(0, 0, o->card.w, o->card.h);
	h_true(lay_inside(screen, o->prompt), "invite dans l'ecran");
	h_true(!lay_overlap(o->prompt, o->card), "invite hors carte");
	h_true(lay_inside(screen, o->chips), "puces dans l'ecran");
	h_true(!lay_overlap(o->chips, o->card), "puces hors carte");
	h_true(lay_inside(local, o->password), "saisie dans la carte");
	h_true(lay_inside(local, o->go), "bouton aller dans la carte");
	h_true(lay_inside(local, o->message), "message dans la carte");
	h_true(lay_inside(local, o->shutdown), "Eteindre dans la carte");
	h_true(!lay_overlap(o->password, o->go), "saisie et bouton disjoints");
	h_true(!lay_overlap(o->message, o->password), "message hors saisie");
	h_true(!lay_overlap(o->shutdown, o->message), "Eteindre hors message");
	h_true(lay_inside(local, o->dlg_text), "texte dans la carte");
	h_true(lay_inside(local, o->dlg_ok), "OK dans la carte");
	h_true(lay_inside(local, o->dlg_cancel), "Annuler dans la carte");
	h_true(!lay_overlap(o->dlg_ok, o->dlg_cancel), "boutons disjoints");
	h_true(!lay_overlap(o->dlg_text, o->dlg_ok), "texte hors boutons");
}

#endif
