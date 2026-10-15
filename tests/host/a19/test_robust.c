#include "harness.h"
#include "fake.h"
#include "ctl_int.h"

static const int	g_x[8] = {0, 0, -50, 190, 5, 0, 300, -5};
static const int	g_y[8] = {0, 0, -50, 110, 5, 0, 300, -5};
static const int	g_w[8] = {0, 1, 30, 100, 2, CTL_COORD_MAX, 5, 3};
static const int	g_hh[8] = {0, 1, 30, 100, 2, CTL_COORD_MAX, 5, 3};

static void	degenerate_rects(void)
{
	t_fakeui	u;
	uint32_t	type;
	int			r;
	t_ctl		*c;

	type = 0;
	while (type < CT_IMAGE)
	{
		r = 0;
		while (r < 8 && type != CT_MENU)
		{
			fake_begin(&u, 200, 120);
			c = fk_c(&u, type, 1, rect_make(g_x[r], g_y[r], g_w[r], g_hh[r]));
			h_true(c != NULL, "creation");
			fake_poke(&u, c);
			fake_done(&u);
			r++;
		}
		type++;
	}
}

static void	resize_sweep(void)
{
	t_fakeui	u;
	uint32_t	type;
	int			size;
	t_ctl		*c;

	type = 0;
	while (type < CT_IMAGE)
	{
		fake_begin(&u, 200, 120);
		c = fk_c(&u, type, 1, rect_make(10, 10, 50, 20));
		size = 0;
		while (c && type != CT_MENU && size < 40)
		{
			ctl_set_rect(&u.r, c, rect_make(10, 10, size, 40 - size));
			fake_poke(&u, c);
			size++;
		}
		fake_done(&u);
		type++;
	}
}

static void	tiny_surfaces(void)
{
	t_fakeui	u;
	int			side;

	side = 0;
	while (side < 4)
	{
		fake_begin(&u, side, side);
		fake_add(&u, fake_spec(CT_BUTTON, 1, rect_make(0, 0, 50, 20), "b"));
		fake_add(&u, fake_spec(CT_EDIT, 2, rect_make(0, 0, 50, 20), "t"));
		fake_add(&u, fake_spec(CT_LIST, 3, rect_make(0, 0, 50, 20), ""));
		ctl_list_add(&u.r, ctl_find(&u.r, 3), "x");
		fake_menu(&u, 0, 0);
		fake_click(&u.r, 0, 0);
		fake_press(&u.r, VK_DOWN, 0);
		fake_press(&u.r, VK_RETURN, 0);
		ctl_paint(&u.r);
		fake_done(&u);
		side++;
	}
}

int	main(void)
{
	h_begin("a19/robust");
	h_run("rectangles degeneres, tous les types", degenerate_rects);
	h_run("redimensionnement de 0 a 40, tous les types", resize_sweep);
	h_run("surfaces de 0x0 a 3x3, menu compris", tiny_surfaces);
	return (h_end());
}
