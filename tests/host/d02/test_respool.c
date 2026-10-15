#include <stdlib.h>
#include "harness.h"
#include "fx.h"

static void	table(const char *name)
{
	t_span		s;
	t_respool	p;
	char		buf[400];
	t_text		t;

	s = fx_load(name);
	t.p = buf;
	t.cap = sizeof(buf);
	h_eq_i64("ouverture", respool_open(&p, s), 0);
	h_eq_i64("ascii", respool_get(&p, 0, t), 3);
	h_eq_str("ascii texte", buf, "abc");
	h_eq_i64("hors plan de base", respool_get(&p, 1, t), 6);
	h_eq_str("hors plan texte", buf, "\xc3\xa9\xf0\x9f\x98\x80");
	h_eq_i64("longueur sur deux octets", respool_get(&p, 2, t), 300);
	h_eq_i64("vide", respool_get(&p, 3, t), 0);
	h_eq_i64("indice hors table", respool_get(&p, 4, t), E_RANGE);
	t.cap = 3;
	h_eq_i64("tampon trop petit", respool_get(&p, 0, t), E_OVERFLOW);
	t.cap = 4;
	h_eq_i64("tampon juste", respool_get(&p, 0, t), 3);
	t.cap = 0;
	h_eq_i64("tampon nul", respool_get(&p, 0, t), E_INVAL);
	fx_free(s);
}

static void	tables(void)
{
	t_span		s;
	t_respool	p;
	t_text		t;

	table("pool_u8.bin");
	table("pool_u16.bin");
	s = fx_load("pool_u16_long.bin");
	t.cap = 0x8001;
	t.p = malloc(t.cap);
	h_eq_i64("longue ouverture", respool_open(&p, s), 0);
	h_eq_i64("longueur sur deux unites", respool_get(&p, 0, t), 0x8000);
	t.cap = 0x8000;
	h_eq_i64("longue trop petit", respool_get(&p, 0, t), E_OVERFLOW);
	free(t.p);
	fx_free(s);
	s = fx_load("pool_u16_lone.bin");
	t.p = (char *)&p.utf8;
	t.cap = 1;
	h_eq_i64("substituts ouverture", respool_open(&p, s), 0);
	h_eq_i64("haut isole", respool_get(&p, 0, t), E_INVAL);
	h_eq_i64("bas isole", respool_get(&p, 1, t), E_INVAL);
	h_eq_i64("haut sans bas", respool_get(&p, 2, t), E_INVAL);
	fx_free(s);
}

static int	patched(const char *name, size_t off, uint8_t v)
{
	t_span		s;
	t_respool	p;
	char		buf[64];
	t_text		t;
	int			r;

	s = fx_load(name);
	t.p = buf;
	t.cap = sizeof(buf);
	fx_poke(s, off, v);
	r = respool_open(&p, s);
	if (r == 0)
		r = respool_get(&p, 0, t);
	fx_free(s);
	return (r);
}

static void	refus(void)
{
	t_span	vide;

	vide.p = NULL;
	vide.len = 0;
	h_eq_i64("span nul", respool_open(NULL, vide), E_INVAL);
	h_eq_i64("type", patched("pool_u8.bin", 0, 2), E_INVAL);
	h_eq_i64("entete court", patched("pool_u8.bin", 2, 27), E_INVAL);
	h_eq_i64("taille", patched("pool_u8.bin", 7, 1), E_INVAL);
	h_eq_i64("compteur", patched("pool_u8.bin", 11, 1), E_INVAL);
	h_eq_i64("debut des chaines", patched("pool_u8.bin", 22, 1), E_INVAL);
	h_eq_i64("decalage", patched("pool_u8.bin", 30, 1), E_INVAL);
	h_eq_i64("utf8 invalide", patched("pool_u8.bin", 46, 0xff), E_INVAL);
	h_eq_i64("utf8 sur-long", patched("pool_u8.bin", 46, 0xc0), E_INVAL);
	h_eq_i64("nul interne", patched("pool_u8.bin", 46, 0), E_INVAL);
	h_eq_i64("fin absente", patched("pool_u8.bin", 49, 'x'), E_INVAL);
	h_eq_i64("longueur u8", patched("pool_u8.bin", 45, 0x7f), E_INVAL);
	h_eq_i64("fin absente u16", patched("pool_u16.bin", 52, 1), E_INVAL);
	h_eq_i64("haut u16", patched("pool_u16.bin", 47, 0xd8), E_INVAL);
	h_eq_i64("bas u16", patched("pool_u16.bin", 47, 0xdc), E_INVAL);
	h_eq_i64("longueur u16", patched("pool_u16.bin", 45, 0x7f), E_INVAL);
	h_eq_i64("temoin sain", patched("pool_u16.bin", 0, 1), 3);
}

int	main(void)
{
	h_begin("d02 respool");
	h_run("respool_tables", tables);
	h_run("respool_refus", refus);
	return (h_end());
}
