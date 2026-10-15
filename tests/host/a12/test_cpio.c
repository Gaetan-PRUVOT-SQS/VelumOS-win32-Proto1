#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "harness.h"
#include "fake.h"
#include "cpio.h"
#include "velum/err.h"
#include "velum/heap.h"

static void	syntaxe_valide(void)
{
	uint8_t		raw[4096];
	t_vbuf		b;
	t_initrd	rd;
	t_vnode		n;
	uint8_t		*c;

	b.p = raw;
	b.len = 0;
	b.cap = sizeof(raw);
	h_eq_i64("construction", fakecpio_valid(&b), 0);
	c = fake_dup(raw, b.len);
	h_eq_i64("index", cpio_index(c, b.len, &rd), 0);
	h_eq_u64("entrées gardées", rd.n, 6);
	h_eq_u64("ignorées (racine . et lien)", rd.skipped, 2);
	h_true(rd_lookup(&rd, "system/bin/hello", &n) == 0 && n.size == 7,
		"doublon : la dernière entrée gagne");
	h_true(rd_lookup(&rd, "vide", &n) == 0 && n.size == 0
		&& (n.mode & S_TYPE_REG), "taille 0");
	h_true(rd_lookup(&rd, "x/y", &n) == 0 && (n.mode & S_TYPE_DIR),
		"dossier implicite");
	h_true(rd_lookup(&rd, "", &n) == 0 && (n.mode & S_TYPE_DIR), "racine");
	h_eq_i64("absent", rd_lookup(&rd, "x/y/zz", &n), E_NOENT);
	h_eq_i64("préfixe non dossier", rd_lookup(&rd, "syst", &n), E_NOENT);
	kfree(rd.ents);
	free(c);
}

static int	mutate(const uint8_t *src, size_t len, size_t at, uint8_t v)
{
	t_initrd	rd;
	uint8_t		*c;
	int			rc;

	c = fake_dup(src, len);
	if (at < len)
		c[at] = v;
	rc = cpio_index(c, len, &rd);
	if (rc == 0)
		kfree(rd.ents);
	free(c);
	return (rc);
}

static void	champs_invalides(void)
{
	uint8_t		raw[4096];
	t_vbuf		b;
	size_t		i;

	b.p = raw;
	b.len = 0;
	b.cap = sizeof(raw);
	fakecpio_valid(&b);
	h_eq_i64("magic faux", mutate(raw, b.len, 0, 'X'), E_INVAL);
	i = 0;
	while (i < 13)
		h_eq_i64("champ non hexa", mutate(raw, b.len, 6 + 8 * i++ + 3, 'g'),
			E_INVAL);
	memcpy(raw + 6 + 11 * 8, "00000000", 8);
	h_eq_i64("namesize 0", mutate(raw, b.len, b.len, 0), E_INVAL);
	memcpy(raw + 6 + 11 * 8, "ffffffff", 8);
	h_eq_i64("namesize géant", mutate(raw, b.len, b.len, 0), E_INVAL);
	memcpy(raw + 6 + 11 * 8, "00000001", 8);
	h_eq_i64("nom non terminé", mutate(raw, b.len, b.len, 0), E_INVAL);
	memcpy(raw + 6 + 11 * 8, "00000002", 8);
	memcpy(raw + 6 + 6 * 8, "7fffffff", 8);
	h_eq_i64("filesize géant", mutate(raw, b.len, b.len, 0), E_INVAL);
}

static void	troncature(void)
{
	uint8_t		raw[4096];
	t_vbuf		b;
	size_t		i;
	int			bad;

	b.p = raw;
	b.len = 0;
	b.cap = sizeof(raw);
	fakecpio_valid(&b);
	h_eq_i64("archive vide acceptée", mutate(raw, 0, 0, 0), 0);
	bad = 0;
	i = 1;
	while (i < b.len - 4)
		bad += (mutate(raw, i++, b.len, 0) != E_INVAL);
	h_eq_i64("chaque préfixe tronqué refusé", bad, 0);
	h_eq_i64("sans TRAILER", mutate(raw, b.len - 124, b.len, 0), E_INVAL);
}

int	main(void)
{
	h_begin("a12/cpio");
	h_run("syntaxe valide, noms longs, taille 0", syntaxe_valide);
	h_run("champs invalides", champs_invalides);
	h_run("troncature à chaque octet", troncature);
	return (h_end());
}
