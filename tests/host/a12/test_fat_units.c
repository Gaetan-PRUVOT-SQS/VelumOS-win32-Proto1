#include <string.h>
#include "harness.h"
#include "fake.h"
#include "fat.h"
#include "velum/err.h"

static void	horodatages(void)
{
	uint16_t	d;
	uint16_t	t;

	fat_time_pack(0, &d, &t);
	h_true(d == ((0 << 9) | (1 << 5) | 1) && t == 0, "1970 -> 1980-01-01");
	fat_time_pack(1709208000ull * 1000000000ull, &d, &t);
	h_true(d == ((44 << 9) | (2 << 5) | 29) && t == (12 << 11),
		"2024-02-29 12:00 (bissextile)");
	fat_time_pack(4354819199ull * 1000000000ull, &d, &t);
	h_true(d == ((127 << 9) | (12 << 5) | 31) && t == ((23 << 11)
			| (59 << 5) | 29), "2107-12-31 23:59:58");
	fat_time_pack(5000000000ull * 1000000000ull, &d, &t);
	h_true(d == ((127 << 9) | (12 << 5) | 31), "après 2107 : borné");
	h_eq_u64("aller-retour", fat_time_unpack((44 << 9) | (2 << 5) | 29,
			12 << 11), 1709208000ull * 1000000000ull);
	h_eq_u64("date nulle", fat_time_unpack(0, 0), 0);
	h_eq_u64("mois 13", fat_time_unpack((1 << 9) | (13 << 5) | 1, 0), 0);
	h_eq_u64("heure 24 ignorée", fat_time_unpack((0 << 9) | (1 << 5) | 1,
			24 << 11), 315532800ull * 1000000000ull);
}

static void	bases_courtes(void)
{
	uint8_t	raw[32];
	int		lossy;

	fat_basis("The quick brown.fox", raw, &lossy);
	h_true(memcmp(raw, "THEQUICKFOX", 11) == 0 && !lossy, "base spec");
	fat_sname_try("The quick brown.fox", raw, 1, raw);
	h_true(memcmp(raw, "THEQUI~1FOX", 11) == 0, "queue ~1 (exemple spec)");
	fat_basis("...a+b,c.tar.gz", raw, &lossy);
	h_true(memcmp(raw, "A_B_C   GZ ", 11) == 0 && lossy, "points, +, ,");
	fat_basis("été.txt", raw, &lossy);
	h_true(memcmp(raw, "_T_     TXT", 11) == 0 && lossy, "non ASCII -> _");
	fat_sname_try("x", raw, 99999, raw);
	h_true(raw[6] == '~' && raw[7] == '1', "variante hachée ~1");
	h_true(fat_fits_83("README.TXT") && fat_fits_83("A") && !fat_fits_83(
			"abcdefghi") && !fat_fits_83("a.b.c") && !fat_fits_83("a.html")
		&& !fat_fits_83(".x") && !fat_fits_83("a b"), "règle 8.3");
}

static void	decodage_court(void)
{
	uint8_t	raw[32];
	char	s[13];

	memset(raw, 0, 32);
	memcpy(raw, "README  TXT", 11);
	raw[12] = 0x18;
	fat_sname_decode(raw, s);
	h_eq_str("NTRes minuscules", s, "readme.txt");
	memcpy(raw, "\x05" "BC     X\x01\x20", 11);
	raw[12] = 0;
	fat_sname_decode(raw, s);
	h_eq_str("0x05 et contrôle", s, "_BC.X_");
}

static void	conversions_utf(void)
{
	uint16_t	u[300];
	char		o[300];
	int			n;

	n = fat_utf8_to16("aé日😀", u, 300);
	h_true(n == 5 && u[1] == 0xE9 && u[2] == 0x65E5 && u[3] == 0xD83D
		&& u[4] == 0xDE00, "UTF-8 -> UTF-16 avec paire");
	h_eq_i64("UTF-16 -> UTF-8", fat_utf16_to8(u, 5, o, 300), 10);
	h_eq_str("aller-retour", o, "aé日😀");
	u[0] = 0xD800;
	h_eq_i64("surrogate seul", fat_utf16_to8(u, 1, o, 300), E_INVAL);
	u[0] = 'a';
	u[1] = '/';
	h_eq_i64("barre dans un LFN", fat_utf16_to8(u, 2, o, 300), E_INVAL);
	h_eq_i64("capacité", fat_utf16_to8(u, 1, o, 1), E_RANGE);
	memset(o, 'a', 299);
	o[256] = '\0';
	h_eq_i64("256 unités > 255", fat_utf8_to16(o, u, 255), E_RANGE);
	h_true(vname_eq("ÉTÉ", "été") && vname_eq("ЖУК", "жук")
		&& vname_eq("ŁÓDŹ", "łódź") && !vname_eq("é", "e"),
		"casse latin, cyrillique, latin étendu");
}

int	main(void)
{
	h_begin("a12/fat_units");
	h_run("horodatages aux limites", horodatages);
	h_run("noms courts : base et queue (spec)", bases_courtes);
	h_run("noms courts : décodage", decodage_court);
	h_run("conversions UTF et casse", conversions_utf);
	return (h_end());
}
