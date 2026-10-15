#include "harness.h"
#include "fake.h"
#include "dex_int.h"

static uint32_t	hdr(uint32_t off)
{
	return (dex_u32(fake_fixture().p + off));
}

static void	refuse(const char *what, uint32_t off, uint32_t v)
{
	h_eq_i64(what, fake_patched(off, v, 4), E_INVAL);
}

static void	header(void)
{
	t_dex	d;

	h_true(fake_fixture().len > 0x70, "fixture lue");
	h_eq_i64("fixture acceptee", dex_open(&d, fake_fixture()), E_OK);
	h_eq_i64("magie", fake_patched(0, 'x', 1), E_INVAL);
	h_eq_i64("version 034", fake_patched(6, '4', 1), E_INVAL);
	h_eq_i64("version 039", fake_patched(6, '9', 1), E_OK);
	h_eq_i64("version 040", fake_patched(5, '4', 1), E_INVAL);
	h_eq_i64("magie non terminee", fake_patched(7, 1, 1), E_INVAL);
	refuse("file_size", 32, hdr(32) + 1);
	refuse("header_size", 36, 0x78);
	h_eq_i64("gros-boutiste", fake_patched(40, 0x78563412, 4), E_NOTSUP);
	refuse("boutisme inconnu", 40, 0);
	h_eq_i64("somme fausse", fake_patched(13, 0xa5, 9), E_INVAL);
	h_eq_i64("somme recalculee", fake_patched(13, 0xa5, 1), E_OK);
	h_eq_i64("pointeur nul", dex_open(0, fake_fixture()), E_INVAL);
	h_eq_i64("copie intacte", fake_patched(0, 0, 0), E_OK);
}

static void	tables(void)
{
	uint32_t	i;

	i = 0;
	while (i < 6)
	{
		refuse("table : taille hors fichier", 56 + 8 * i, 0x10000000);
		refuse("table : decalage hors fichier", 60 + 8 * i, hdr(32));
		refuse("table : non alignee", 60 + 8 * i, hdr(60 + 8 * i) + 1);
		refuse("table : dans l'en-tete", 60 + 8 * i, 0x40);
		i++;
	}
	refuse("types > 65535", 64, 0x10000);
	refuse("data hors fichier", 104, hdr(32));
	refuse("link hors fichier", 44, hdr(32) + 1);
	refuse("map hors fichier", 52, hdr(32));
	refuse("map non alignee", 52, hdr(52) + 1);
	refuse("map trop longue", hdr(52), 0x10000000);
}

int	main(void)
{
	h_begin("d03/open");
	h_run("en-tete : chaque verification a son refus", header);
	h_run("tables et zones dans le fichier", tables);
	return (h_end());
}
