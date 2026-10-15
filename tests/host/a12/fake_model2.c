#include <stdlib.h>
#include <string.h>
#include "fake.h"
#include "velum/err.h"

void	model_rename(t_model *m, t_mfile *a, t_mfile *b)
{
	uint8_t		*tmp;
	int			want;

	want = 0;
	if (!a->exists)
		want = E_NOENT;
	else if (b->exists && a != b)
		want = E_EXIST;
	model_expect(m, vfs_rename(a->path, b->path), want);
	if (want || a == b)
		return ;
	tmp = b->data;
	b->data = a->data;
	a->data = tmp;
	b->size = a->size;
	b->exists = 1;
	a->exists = 0;
	a->size = 0;
}

static void	model_create(t_model *m, t_mfile *f)
{
	t_vfile	*h;
	int		rc;

	rc = vfs_open(f->path, O_CREAT | O_EXCL | O_WRONLY, &h);
	if (f->exists)
		model_expect(m, rc, E_EXIST);
	else
		model_expect(m, rc, 0);
	if (rc == 0)
		vfs_close(h);
	f->exists = 1;
}

static void	model_unlink(t_model *m, t_mfile *f)
{
	int	want;

	want = 0;
	if (!f->exists)
		want = E_NOENT;
	model_expect(m, vfs_unlink(f->path), want);
	f->exists = 0;
	f->size = 0;
}

void	model_step(t_model *m)
{
	t_mfile		*f;
	uint32_t	op;

	m->ops++;
	f = &m->f[fake_rand(&m->seed) % MODEL_FILES];
	op = fake_rand(&m->seed) % 10;
	if (op < 2)
		model_create(m, f);
	else if (op < 5 && f->exists)
		model_write(m, f);
	else if (op == 5 && f->exists)
		model_trunc(m, f);
	else if (op == 6)
		model_unlink(m, f);
	else if (op == 7)
		model_rename(m, f, &m->f[fake_rand(&m->seed) % MODEL_FILES]);
	else if (f->exists)
		model_expect(m, vst_same(f->path, f->data, f->size), 0);
}

int	model_check(t_model *m)
{
	t_vstat		st;
	uint32_t	i;
	int			before;

	before = m->errs;
	i = 0;
	while (i < MODEL_FILES)
	{
		if (m->f[i].exists)
			model_expect(m, vst_same(m->f[i].path, m->f[i].data,
					m->f[i].size), 0);
		else
			model_expect(m, vfs_stat(m->f[i].path, &st), E_NOENT);
		i++;
	}
	return (m->errs - before);
}
