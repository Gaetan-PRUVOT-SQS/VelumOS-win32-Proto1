#include <string.h>
#include "fake.h"
#include "ctl_int.h"

static const uint32_t	g_cps[12] = {'a', 'Z', ' ', '@', 0xe9, 0x20ac,
	0x1f600, 0x10ffff, 0x7f, 0xd800, 0x110000, 0x4e2d};
static const char		*g_words[6] = {"", "abc",
	"\xc3\xa9\xe2\x82\xac\xf0\x9f\x98\x80", "hello w\xc3\xb6rld",
	"\xf0\x9f\x98\x80\xf0\x9f\x98\x80", "z"};

static void	fuzz_keys(t_fakeui *u)
{
	uint32_t	k;
	uint32_t	mods;

	k = fake_rand() % 12;
	mods = 0;
	if (fake_rand() % 3 == 0)
		mods = INPM_SHIFT;
	if (k < 2)
		fake_press(&u->r, VK_BACK, 0);
	else if (k < 4)
		fake_press(&u->r, VK_DELETE, 0);
	else if (k < 6)
		fake_press(&u->r, VK_LEFT, mods);
	else if (k < 8)
		fake_press(&u->r, VK_RIGHT, mods);
	else if (k == 8)
		fake_press(&u->r, VK_HOME, mods);
	else if (k == 9)
		fake_press(&u->r, VK_END, mods);
	else
		fake_press(&u->r, "ACXV"[fake_rand() % 4], INPM_CTRL);
}

static void	fuzz_state(t_fakeui *u, t_ctl *e)
{
	uint32_t	k;

	k = fake_rand() % 5;
	if (k == 0)
		ctl_edit_set_limit(&u->r, e, 1 + fake_rand() % 255);
	else if (k == 1)
		ctl_set_text(&u->r, e, g_words[fake_rand() % 6]);
	else if (k == 2)
		ctl_set_rect(&u->r, e, rect_make(5, 5, (int)(fake_rand() % 160), 20));
	else if (k == 3)
		fake_click(&u->r, (int)(fake_rand() % 170), 15);
	else
		fake_move(&u->r, (int)(fake_rand() % 170), 15);
}

void	fake_fuzz_step(t_fakeui *u, t_ctl *e)
{
	uint32_t	k;

	k = fake_rand() % 24;
	if (k < 12)
		fake_char(&u->r, g_cps[fake_rand() % 12]);
	else if (k < 21)
		fuzz_keys(u);
	else
		fuzz_state(u, e);
}

bool	fake_fuzz_ok(const t_ctl *e)
{
	const t_edit	*ed;
	uint32_t		len;

	ed = e->priv;
	len = (uint32_t)strlen(e->text);
	if (len > 255 || ed->caret > len || ed->anchor > len || ed->scroll < 0)
		return (false);
	if (!fake_utf8_boundary(e->text, ed->caret))
		return (false);
	if (!fake_utf8_boundary(e->text, ed->anchor))
		return (false);
	if (fake_utf8_chars(e->text) > ed->max_chars)
		return (false);
	return (fake_utf8_valid(e->text));
}
