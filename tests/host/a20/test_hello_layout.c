#include <stdint.h>
#include "harness.h"
#include "../../../user/apps/hello/hello_layout.h"

static void	hello_fenetre_de_la_maquette(void)
{
	h_eq_u64("x de la fenetre (330)", HELLO_X, 330);
	h_eq_u64("y de la fenetre (160)", HELLO_Y, 160);
	h_eq_u64("largeur 480", HELLO_W, 480);
	h_eq_u64("hauteur: 395 - 160", HELLO_H_PX, 235);
	h_eq_u64("barre de titre 50", VT_CAPTION_H, 50);
	h_eq_u64("client = fenetre - barre", HELLO_CLIENT_H,
		HELLO_H_PX - VT_CAPTION_H);
}

static void	hello_bouton_et_marges(void)
{
	h_eq_u64("anneau de focus de lib/ctl", LUNA_FOCUS_MARGIN, 4);
	h_eq_u64("capsule x: 688 - 330", HELLO_CAPSULE_X, 358);
	h_eq_u64("capsule y: 329 - 210", HELLO_CAPSULE_Y, 119);
	h_eq_u64("capsule largeur 96", HELLO_CAPSULE_W, 96);
	h_eq_u64("capsule hauteur 40", HELLO_CAPSULE_H, 40);
	h_eq_u64("bouton x: capsule - 4", HELLO_BTN_X, 354);
	h_eq_u64("bouton y: capsule - 4", HELLO_BTN_Y, 115);
	h_eq_u64("bouton largeur 96 + 8", HELLO_BTN_W, 104);
	h_eq_u64("bouton hauteur 40 + 8", HELLO_BTN_H, 48);
	h_eq_u64("marge gauche: 356 - 330", HELLO_MARGIN, 26);
	h_eq_u64("marge droite de la capsule: 810 - 784",
		HELLO_W - (HELLO_BTN_X + 4 + HELLO_BTN_W - 8), 26);
	h_eq_u64("marge basse de la capsule: 395 - 369",
		HELLO_CLIENT_H - (HELLO_BTN_Y + 4 + HELLO_BTN_H - 8), 26);
	h_eq_u64("largeur du texte", HELLO_TEXT_W, HELLO_W - 2 * 26);
}

static void	hello_ordre_vertical_sans_chevauchement(void)
{
	h_true(HELLO_TITLE_Y + HELLO_TITLE_H <= HELLO_UPTIME_Y,
		"titre au-dessus du temps de fonctionnement");
	h_true(HELLO_UPTIME_Y + HELLO_LINE_H <= HELLO_NOTE_Y,
		"temps de fonctionnement au-dessus de la note");
	h_true(HELLO_NOTE_Y + HELLO_LINE_H <= HELLO_BTN_Y,
		"note au-dessus du bouton");
	h_true(HELLO_MARGIN + HELLO_TEXT_W <= HELLO_W,
		"les textes tiennent dans la largeur");
}

static void	hello_polices_de_la_maquette(void)
{
	h_eq_u64("titre: Instrument Sans 24 gras", HELLO_FONT_TITLE,
		FONT_INS_BOLD_24);
	h_eq_u64("temps de fonctionnement: 14", HELLO_FONT_UPTIME, FONT_INS_14);
	h_eq_u64("note: 13", HELLO_FONT_NOTE, FONT_INS_13);
	h_eq_u64("bouton: 13 semi-gras", HELLO_FONT_BUTTON, FONT_INS_SEMI_13);
	h_true(HELLO_FONT_TITLE < FONT_IDS && HELLO_FONT_UPTIME < FONT_IDS
		&& HELLO_FONT_NOTE < FONT_IDS && HELLO_FONT_BUTTON < FONT_IDS,
		"les polices de hello sont dans FONT_IDS");
	h_true(HELLO_FONT_TITLE >= FONT_INS_11 && HELLO_FONT_UPTIME >= FONT_INS_11
		&& HELLO_FONT_NOTE >= FONT_INS_11 && HELLO_FONT_BUTTON >= FONT_INS_11,
		"aucune police d'ancien style");
}

int	main(void)
{
	h_begin("a20/hello-layout");
	h_run("hello: fenetre 480 de la maquette", hello_fenetre_de_la_maquette);
	h_run("hello: bouton et marges", hello_bouton_et_marges);
	h_run("hello: ordre vertical", hello_ordre_vertical_sans_chevauchement);
	h_run("hello: polices de la maquette", hello_polices_de_la_maquette);
	return (h_end());
}
