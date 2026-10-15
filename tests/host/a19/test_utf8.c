#include <string.h>
#include "harness.h"
#include "fake.h"
#include "ctl_int.h"

static void	len_partitions(void)
{
	h_eq_i64("ascii", ctl_u8_len("a", 1), 1);
	h_eq_i64("2 octets", ctl_u8_len("\xc3\xa9", 2), 2);
	h_eq_i64("3 octets", ctl_u8_len("\xe2\x82\xac", 3), 3);
	h_eq_i64("4 octets", ctl_u8_len("\xf0\x9f\x98\x80", 4), 4);
	h_eq_i64("taille nulle", ctl_u8_len("a", 0), 0);
	h_eq_i64("sur-longueur C0 80", ctl_u8_len("\xc0\x80", 2), 1);
	h_eq_i64("sur-longueur C1 BF", ctl_u8_len("\xc1\xbf", 2), 1);
	h_eq_i64("sur-longueur E0 80 80", ctl_u8_len("\xe0\x80\x80", 3), 1);
	h_eq_i64("sur-longueur F0 80 80 80", ctl_u8_len("\xf0\x80\x80\x80", 4), 1);
	h_eq_i64("substitut ED A0 80", ctl_u8_len("\xed\xa0\x80", 3), 1);
	h_eq_i64("substitut ED BF BF", ctl_u8_len("\xed\xbf\xbf", 3), 1);
	h_eq_i64("au dela de 10FFFF F4 90", ctl_u8_len("\xf4\x90\x80\x80", 4), 1);
	h_eq_i64("F5 invalide", ctl_u8_len("\xf5\x80\x80\x80", 4), 1);
	h_eq_i64("continuation seule", ctl_u8_len("\x80", 1), 1);
	h_eq_i64("tronque 2/3", ctl_u8_len("\xe2\x82", 2), 1);
	h_eq_i64("tronque 3/4", ctl_u8_len("\xf0\x9f\x98", 3), 1);
	h_eq_i64("limite haute F4 8F BF BF", ctl_u8_len("\xf4\x8f\xbf\xbf", 4), 4);
}

static const uint32_t	g_cp[10] = {0x7f, 0x80, 0x7ff, 0x800, 0xd7ff, 0xe000,
	0xffff, 0x10000, 0x10ffff, 0};
static const uint32_t	g_n[10] = {1, 2, 2, 3, 3, 3, 3, 4, 4, 1};

static void	encode_limits(void)
{
	char	b[5];
	int		i;

	i = 0;
	while (i < 10)
	{
		memset(b, 0, sizeof(b));
		h_eq_u64("longueur encodee", ctl_u8_encode(g_cp[i], b), g_n[i]);
		h_eq_u64("aller-retour", ctl_u8_len(b, 4), g_n[i]);
		i++;
	}
}

static void	encode_invalid(void)
{
	char	b[5];

	h_eq_u64("D800 refuse", ctl_u8_encode(0xd800, b), 0);
	h_eq_u64("DFFF refuse", ctl_u8_encode(0xdfff, b), 0);
	h_eq_u64("110000 refuse", ctl_u8_encode(0x110000, b), 0);
	h_eq_u64("FFFFFFFF refuse", ctl_u8_encode(0xffffffffu, b), 0);
}

static void	next_prev_count(void)
{
	const char	*s;
	uint32_t	pos;
	uint32_t	n;

	s = "a\xc3\xa9\xe2\x82\xac\xf0\x9f\x98\x80z";
	h_eq_u64("nombre de caracteres", ctl_u8_count(s, 11), 5);
	pos = 0;
	n = 0;
	while (pos < 11)
	{
		h_eq_u64("prev(next(p)) = p", ctl_u8_prev(s, ctl_u8_next(s, 11, pos)),
			pos);
		h_true(fake_utf8_boundary(s, pos), "frontiere");
		pos = ctl_u8_next(s, 11, pos);
		n++;
	}
	h_eq_u64("5 pas", n, 5);
	h_eq_u64("next au bord", ctl_u8_next(s, 11, 11), 11);
	h_eq_u64("prev au debut", ctl_u8_prev(s, 0), 0);
	h_eq_u64("prev octet invalide", ctl_u8_prev("\x80\x80", 2), 1);
}

int	main(void)
{
	h_begin("a19/utf8");
	h_run("decodage strict : partitions et limites", len_partitions);
	h_run("encodage : valeurs limites", encode_limits);
	h_run("encodage : points de code invalides", encode_invalid);
	h_run("next, prev, count : aller-retour", next_prev_count);
	return (h_end());
}
