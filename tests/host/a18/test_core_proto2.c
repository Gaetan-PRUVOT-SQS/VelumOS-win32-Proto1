#include <string.h>
#include "harness.h"
#include "help.h"

static void	create(t_wmcreate *c)
{
	memset(c, 0, sizeof(*c));
	ws_hdr(&c->h, WMC_CREATE, sizeof(*c), 0);
	c->rect = rect_make(-WS_COORD_MAX, WS_COORD_MAX, 1, WS_DIM_MAX);
	c->style = WS_STYLE_KNOWN;
	memcpy(c->title, "D\xc3\xa9marrer \xe2\x82\xac", 13);
}

static void	create_rect_style(void)
{
	t_wmcreate	c;

	create(&c);
	h_eq_i64("create aux limites", hv_check(&c, sizeof(c)), 0);
	c.rect.w = 0;
	h_eq_i64("R14 largeur 0", hv_check(&c, sizeof(c)), E_PROTO);
	c.rect.w = WS_DIM_MAX + 1;
	h_eq_i64("R14 largeur max + 1", hv_check(&c, sizeof(c)), E_PROTO);
	create(&c);
	c.rect.x = -WS_COORD_MAX - 1;
	h_eq_i64("R14 x min - 1", hv_check(&c, sizeof(c)), E_PROTO);
	create(&c);
	c.rect.y = WS_COORD_MAX + 1;
	h_eq_i64("R14 y max + 1", hv_check(&c, sizeof(c)), E_PROTO);
	create(&c);
	c.style = WS_STYLE_KNOWN + 1;
	h_eq_i64("R14 style inconnu", hv_check(&c, sizeof(c)), E_PROTO);
	create(&c);
	c.state = 4;
	h_eq_i64("R14 etat", hv_check(&c, sizeof(c)), E_PROTO);
}

static void	create_title(void)
{
	t_wmcreate	c;

	create(&c);
	memset(c.title, 'A', sizeof(c.title));
	h_eq_i64("R14 titre non termine", hv_check(&c, sizeof(c)), E_PROTO);
	c.title[WM_TITLE_MAX - 1] = '\0';
	h_eq_i64("titre de 63 octets", hv_check(&c, sizeof(c)), 0);
	memcpy(c.title, "\xc0\x80", 3);
	h_eq_i64("R14 titre sur-long", hv_check(&c, sizeof(c)), E_PROTO);
	memcpy(c.title, "a\tb", 4);
	h_eq_i64("R14 titre avec tabulation", hv_check(&c, sizeof(c)), E_PROTO);
}

static void	present(void)
{
	t_wmpresent	p;

	memset(&p, 0, sizeof(p));
	ws_hdr(&p.h, WMC_PRESENT, sizeof(p), 9);
	h_eq_i64("present sans rectangle", hv_check(&p, sizeof(p)), 0);
	p.nrects = WM_RECTS_MAX;
	p.rects[15] = rect_make(WS_DIM_MAX, WS_DIM_MAX, WS_DIM_MAX, 0);
	h_eq_i64("16 rectangles aux limites", hv_check(&p, sizeof(p)), 0);
	p.nrects = WM_RECTS_MAX + 1;
	h_eq_i64("R15 17 rectangles", hv_check(&p, sizeof(p)), E_PROTO);
	p.nrects = 1;
	p.rects[0] = rect_make(-1, 0, 5, 5);
	h_eq_i64("R15 x negatif", hv_check(&p, sizeof(p)), E_PROTO);
	p.rects[0] = rect_make(0, 0, 5, -1);
	h_eq_i64("R15 hauteur negative", hv_check(&p, sizeof(p)), E_PROTO);
	p.rects[0] = rect_make(0, 0, WS_DIM_MAX + 1, 1);
	h_eq_i64("R15 largeur geante", hv_check(&p, sizeof(p)), E_PROTO);
	h_eq_i64("R10 longueur - 1", hv_check(&p, sizeof(p) - 1), E_PROTO);
}

int	main(void)
{
	h_begin("a18/core_proto2");
	h_run("creation rectangle et style", create_rect_style);
	h_run("creation titre", create_title);
	h_run("presentation", present);
	return (h_end());
}
