#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fake.h"
#include "velum/err.h"
#include "velum/heap.h"

void	fake_le(uint8_t *p, uint32_t size, uint32_t v)
{
	uint32_t	i;

	i = 0;
	while (i < size)
	{
		p[i] = (uint8_t)(v >> (8 * i));
		i++;
	}
}

static int	put_file(const char *path, uint32_t len)
{
	uint8_t		*data;
	uint32_t	i;
	int			rc;

	data = malloc(len + 1);
	i = 0;
	while (i < len)
	{
		data[i] = (uint8_t)(i * 13 + len);
		i++;
	}
	rc = vst_put(path, data, len);
	free(data);
	return (rc);
}

int	fake_populate(const char *root)
{
	char	p[VFS_PATH_MAX];
	int		rc;

	snprintf(p, sizeof(p), "%s/a.txt", root);
	rc = put_file(p, 100);
	snprintf(p, sizeof(p), "%s/Grand fichier.bin", root);
	if (rc == 0)
		rc = put_file(p, 1500);
	snprintf(p, sizeof(p), "%s/sous", root);
	if (rc == 0)
		rc = vfs_mkdir(p, 0);
	snprintf(p, sizeof(p), "%s/sous/Fichier imbriqué.txt", root);
	if (rc == 0)
		rc = put_file(p, 700);
	return (rc);
}

static void	walk_one(const char *dir, const t_dirent *d)
{
	char		p[VFS_PATH_MAX * 2 + 2];
	void		*buf;
	size_t		n;
	t_vfile		*f;
	t_dirent	sub;

	snprintf(p, sizeof(p), "%s/%s", dir, d->name);
	if (d->type == S_TYPE_REG && vfs_read_all(p, &buf, &n) == 0)
		kfree(buf);
	if (d->type == S_TYPE_DIR && vfs_open(p, O_DIRECTORY, &f) == 0)
	{
		while (vfs_readdir(f, &sub) == 1)
			;
		vfs_close(f);
	}
}

int	fake_walk(const char *dir)
{
	t_vfile		*f;
	t_dirent	d;
	int			n;

	if (vfs_open(dir, O_DIRECTORY, &f) < 0)
		return (-1);
	n = 0;
	while (n < 200 && vfs_readdir(f, &d) == 1)
	{
		walk_one(dir, &d);
		n++;
	}
	vfs_close(f);
	return (n);
}
