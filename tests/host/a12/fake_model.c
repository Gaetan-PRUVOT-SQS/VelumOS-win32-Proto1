#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fake.h"
#include "velum/err.h"

static const char	*g_names[] = {"a.txt", "Fichier long numéro un.dat",
	"README", "ÉtéÀ.bin", "x", "Données de test très longues 2026.log",
	"b.c", "Mixte.Case.Name.tar", "dernier", "UN.TXT", "z9", "Longue~1"};

void	model_init(t_model *m, uint64_t seed)
{
	uint32_t	i;
	const char	*sub;

	memset(m, 0, sizeof(*m));
	m->seed = seed;
	m->buf = malloc(MODEL_MAX + 8192);
	i = 0;
	while (i < MODEL_FILES)
	{
		sub = "";
		if (i >= 12)
			sub = "Sous dossier/";
		snprintf(m->f[i].path, sizeof(m->f[i].path), "/d/%s%s", sub,
			g_names[i % 12]);
		m->f[i].data = calloc(MODEL_MAX, 1);
		i++;
	}
}

void	model_free(t_model *m)
{
	uint32_t	i;

	i = 0;
	while (i < MODEL_FILES)
		free(m->f[i++].data);
	free(m->buf);
}

void	model_expect(t_model *m, int64_t got, int64_t want)
{
	if (got == want)
		return ;
	m->errs++;
	if (m->errs < 10)
		fprintf(stderr, "modèle : op %llu obtenu %lld attendu %lld\n",
			(unsigned long long)m->ops, (long long)got, (long long)want);
}

void	model_write(t_model *m, t_mfile *f)
{
	t_vfile		*h;
	uint32_t	off;
	uint32_t	len;
	uint32_t	i;

	off = fake_rand(&m->seed) % (f->size + 3000);
	len = fake_rand(&m->seed) % 6000;
	if (off > MODEL_MAX - 6000)
		off = MODEL_MAX - 6000;
	i = 0;
	while (i < len)
		m->buf[i++] = (uint8_t)fake_rand(&m->seed);
	model_expect(m, vfs_open(f->path, O_RDWR, &h), 0);
	model_expect(m, vfs_pwrite(h, m->buf, len, off), len);
	vfs_close(h);
	if (len && off > f->size)
		memset(f->data + f->size, 0, off - f->size);
	memcpy(f->data + off, m->buf, len);
	if (len && off + len > f->size)
		f->size = off + len;
}

void	model_trunc(t_model *m, t_mfile *f)
{
	t_vfile		*h;
	uint32_t	size;

	size = fake_rand(&m->seed) % MODEL_MAX;
	model_expect(m, vfs_open(f->path, O_WRONLY, &h), 0);
	model_expect(m, vfs_truncate(h, size), 0);
	vfs_close(h);
	if (size > f->size)
		memset(f->data + f->size, 0, size - f->size);
	f->size = size;
}
