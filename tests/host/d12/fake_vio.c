#include <string.h>
#include "velum/err.h"
#include "velum/vfile.h"
#include "velum/vobj.h"
#include "fake.h"

int64_t	v_open(const char *path, uint32_t flags, uint32_t mode)
{
	int	i;

	(void)mode;
	if (fv_tick() < 0)
		return (E_IO);
	i = fv_find(path);
	if (i < 0 && !(flags & O_CREAT))
		return (E_NOENT);
	if (i < 0 || (flags & O_TRUNC))
		i = fv_put(path, "");
	if (i < 0)
		return (i);
	g_fv.f[i].pos = 0;
	return (i + 1);
}

int64_t	v_read(t_handle file, void *buf, size_t len)
{
	t_fvfile	*f;
	size_t		n;

	if (file == 0 || file > FV_FILES || !g_fv.f[file - 1].used)
		return (E_BADF);
	f = &g_fv.f[file - 1];
	n = f->len - f->pos;
	if (n > len)
		n = len;
	if (n > FV_READ_STEP)
		n = FV_READ_STEP;
	memcpy(buf, f->data + f->pos, n);
	f->pos += n;
	return ((int64_t)n);
}

int64_t	v_write(t_handle file, const void *buf, size_t len)
{
	t_fvfile	*f;

	if (file == 0 || file > FV_FILES || !g_fv.f[file - 1].used)
		return (E_BADF);
	if (fv_tick() < 0)
		return (E_IO);
	f = &g_fv.f[file - 1];
	if (len > FV_WRITE_STEP)
		len = FV_WRITE_STEP;
	if (f->len + len > FV_DATA)
		return (E_NOSPC);
	memcpy(f->data + f->len, buf, len);
	f->len += len;
	return ((int64_t)len);
}

int	v_close(t_handle h)
{
	if (h == 0 || h > FV_FILES)
		return (E_BADF);
	return (0);
}

int	v_fstat(t_handle file, t_vstat *out)
{
	if (file == 0 || file > FV_FILES || !g_fv.f[file - 1].used)
		return (E_BADF);
	memset(out, 0, sizeof(*out));
	out->size = g_fv.f[file - 1].len;
	return (0);
}
