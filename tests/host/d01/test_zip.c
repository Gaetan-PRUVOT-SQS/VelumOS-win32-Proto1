#include <stdlib.h>
#include "harness.h"
#include "d01.h"

static void	cas_ouverture(void)
{
	t_blob	pack;
	t_rec	r;
	size_t	off;
	uint8_t	*in;
	t_zip	z;

	pack = d01_load("cas.pack");
	off = 0;
	while (d01_next(&pack, &off, &r))
	{
		in = d01_dup(r.a, r.alen);
		h_eq_i64("zip_open", zip_open(&z, d01_span(in, r.alen)), r.code);
		h_eq_i64("parcours", d01_walk(&z), 0);
		free(in);
	}
	free(pack.p);
}

static void	contenu_entree(const t_zip *z, const t_rec *r)
{
	t_zipent	e;
	char		name[256];
	uint8_t		*out;
	size_t		i;

	i = 0;
	while (i < r->alen)
	{
		name[i] = (char)r->a[i];
		i++;
	}
	name[i] = '\0';
	h_eq_i64("zip_find", zip_find(z, name, &e), 0);
	h_eq_i64("methode", e.method, r->code);
	h_eq_i64("name_len", e.name_len, (int64_t)r->alen);
	h_eq_i64("nom sur", zip_name_safe(e.name, e.name_len), 1);
	out = malloc(r->blen);
	h_eq_i64("zip_extract", zip_extract(z, &e, out, r->blen),
		(int64_t)r->blen);
	h_true(d01_same(out, r->b, r->blen), "contenu");
	if (r->blen > 0)
		h_eq_i64("cap - 1", zip_extract(z, &e, out, r->blen - 1), E_OVERFLOW);
	free(out);
}

static void	contenu(void)
{
	t_blob	zip;
	t_blob	pack;
	t_rec	r;
	size_t	off;
	t_zip	z;

	zip = d01_load("ok.zip");
	pack = d01_load("ok.pack");
	h_eq_i64("zip_open", zip_open(&z, d01_span(zip.p, zip.len)), 0);
	h_eq_i64("count", z.count, 5);
	off = 0;
	while (d01_next(&pack, &off, &r))
		contenu_entree(&z, &r);
	h_eq_i64("absent", zip_find(&z, "absent", (t_zipent *)&r), E_NOENT);
	h_eq_i64("prefixe", zip_find(&z, "a.tx", (t_zipent *)&r), E_NOENT);
	h_eq_i64("indice", zip_entry(&z, 5, (t_zipent *)&r), E_RANGE);
	free(zip.p);
	free(pack.p);
}

static void	crc_et_noms(void)
{
	t_blob		zip;
	t_zip		z;
	t_zipent	e;
	uint8_t		out[256];

	h_eq_u64("crc", zip_crc32(0, (const uint8_t *)"123456789", 9),
		0xcbf43926);
	zip = d01_load("ok.zip");
	h_eq_i64("zip_open", zip_open(&z, d01_span(zip.p, zip.len)), 0);
	h_eq_i64("zip_find", zip_find(&z, "dir/b.bin", &e), 0);
	zip.p[e.data_off + 7] ^= 1;
	h_eq_i64("crc faux", zip_extract(&z, &e, out, 256), E_INVAL);
	h_eq_i64("absolu", zip_name_safe("/a", 2), 0);
	h_eq_i64("parent", zip_name_safe("a/../b", 6), 0);
	h_eq_i64("parent seul", zip_name_safe("..", 2), 0);
	h_eq_i64("parent fin", zip_name_safe("a/..", 4), 0);
	h_eq_i64("antislash", zip_name_safe("a\\b", 3), 0);
	h_eq_i64("nul", zip_name_safe("a\0b", 3), 0);
	h_eq_i64("vide", zip_name_safe("", 0), 0);
	h_eq_i64("points", zip_name_safe("a..b/c...", 9), 1);
	h_eq_i64("simple", zip_name_safe("res/a.xml", 9), 1);
	free(zip.p);
}

int	main(void)
{
	h_begin("d01 zip");
	h_run("cas_ouverture_limites_et_refus", cas_ouverture);
	h_run("contenu_reference", contenu);
	h_run("crc_et_noms", crc_et_noms);
	return (h_end());
}
