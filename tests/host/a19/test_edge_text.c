#include <string.h>
#include "harness.h"
#include "fake.h"
#include "ctl_int.h"

static void	strip_cut_on_boundary(void)
{
	char	src[CTL_TEXT_MAX + 8];
	char	out[CTL_TEXT_MAX];
	int32_t	mn;

	memset(src, 'a', sizeof(src));
	memcpy(src + 254, "\xe2\x82\xac", 3);
	src[257] = '\0';
	h_eq_i64("euro a cheval : coupe avant", ctl_text_strip(src, out, &mn),
		254);
	h_true(fake_utf8_valid(out), "euro a cheval : sortie valide");
	memcpy(src + 253, "\xc3\xa9", 2);
	src[255] = '\0';
	h_eq_i64("e accent entier : garde", ctl_text_strip(src, out, &mn), 255);
	memcpy(src + 252, "\xf0\x9f\x98\x80", 4);
	src[256] = '\0';
	h_eq_i64("4 octets a cheval : coupe avant", ctl_text_strip(src, out, &mn),
		252);
	h_true(fake_utf8_valid(out), "4 octets a cheval : sortie valide");
}

static void	strip_marker_then_cut(void)
{
	char	src[CTL_TEXT_MAX + 8];
	char	out[CTL_TEXT_MAX];
	int32_t	mn;

	src[0] = '&';
	memset(src + 1, 'a', 253);
	memcpy(src + 254, "\xe2\x82\xac", 3);
	src[257] = '\0';
	h_eq_i64("marqueur puis euro a cheval", ctl_text_strip(src, out, &mn),
		253);
	h_eq_i64("mnemonique en tete", mn, 0);
	h_true(fake_utf8_valid(out), "sortie valide");
}

static void	ellipsis_needs_room(void)
{
	t_fakeui	u;
	t_ctl		*l;
	int			i;
	t_rect		box;

	fake_begin(&u, 200, 60);
	box = rect_make(10, 10, FAKE_CHAR_W - 1, 14);
	l = fake_add(&u, fake_spec(CT_LABEL, 1, box, "abcdef"));
	ctl_set_flag(&u.r, l, CTL_ELLIPSIS, true);
	fake_log_clear();
	ctl_paint(&u.r);
	i = 0;
	while (i < fake_log_count())
	{
		if (fake_log_get(i)->kind == FK_TEXT)
			h_true(fake_log_get(i)->r.x + fake_log_get(i)->r.w
				<= box.x + box.w, "ellipse : dans la boite");
		i++;
	}
	fake_done(&u);
}

int	main(void)
{
	h_begin("a19/edge-text");
	h_run("retrait des & : coupe a la frontiere utf-8 de 255",
		strip_cut_on_boundary);
	h_run("marqueur & puis coupe de 255", strip_marker_then_cut);
	h_run("ellipse : jamais plus large que la boite", ellipsis_needs_room);
	return (h_end());
}
