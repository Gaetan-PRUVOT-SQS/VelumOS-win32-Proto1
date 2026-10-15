#include "harness.h"
#include "fake.h"
#include "dex_int.h"

static uint8_t	g_big[20000];

static uint32_t	adler_ref(const uint8_t *p, size_t n)
{
	uint32_t	a;
	uint32_t	b;
	size_t		i;

	a = 1;
	b = 0;
	i = 0;
	while (i < n)
	{
		a = (a + p[i++]) % 65521;
		b = (b + a) % 65521;
	}
	return ((b << 16) | a);
}

static void	adler(void)
{
	size_t	i;

	i = 0;
	while (i < sizeof(g_big))
		g_big[i++] = 0xff;
	h_eq_u64("vide", dex_adler32(g_big, 0), 1);
	h_eq_u64("Wikipedia", dex_adler32((const uint8_t *)"Wikipedia", 9),
		0x11e60398);
	h_eq_u64("5552 octets", dex_adler32(g_big, 5552), adler_ref(g_big, 5552));
	h_eq_u64("5553 octets", dex_adler32(g_big, 5553), adler_ref(g_big, 5553));
	h_eq_u64("20000 octets", dex_adler32(g_big, 20000),
		adler_ref(g_big, 20000));
}

static void	ulebp1(void)
{
	t_dexcur	c;

	c.p = (const uint8_t *)"\x00\x01";
	c.len = 2;
	c.pos = 0;
	c.err = 0;
	h_eq_u64("p1 de 0 : aucun indice", dex_ulebp1(&c), DEX_NO_INDEX);
	h_eq_u64("p1 de 1 : indice 0", dex_ulebp1(&c), 0);
	dex_ulebp1(&c);
	h_eq_i64("p1 hors tampon", c.err, 1);
}

static void	nulls(void)
{
	t_dex		d;
	t_span		s;
	t_dexstr	str;

	s.p = 0;
	s.len = 0;
	h_eq_i64("fichier nul", dex_open(&d, s), E_INVAL);
	h_eq_i64("t_dex refuse : aucune chaine", dex_string(&d, 0, &str),
		E_RANGE);
	h_eq_i64("t_dex refuse : aucune classe", dex_find_class(&d, "LA;"),
		E_NOENT);
	h_eq_i64("descripteur nul", dex_find_class(&d, 0), E_INVAL);
	h_eq_u64("liste nulle", dex_list_at(0, 0), DEX_NO_INDEX);
}

int	main(void)
{
	h_begin("d03/misc");
	h_run("Adler-32 : valeurs connues et blocs de 5552", adler);
	h_run("ULEB128p1", ulebp1);
	h_run("arguments nuls et lecteur refuse", nulls);
	return (h_end());
}
