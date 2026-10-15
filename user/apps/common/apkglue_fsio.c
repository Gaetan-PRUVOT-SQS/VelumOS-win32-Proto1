#include "velum/err.h"
#include "velum/libk.h"
#include "velum/vfile.h"
#include "velum/vobj.h"
#include "apkglue.h"

static int64_t	read_into(const char *path, t_text out)
{
	int64_t	h;
	int64_t	n;
	size_t	total;
	char	extra;

	h = v_open(path, O_RDONLY, 0);
	if (h < 0)
		return (h);
	total = 0;
	n = 1;
	while (n > 0 && total < out.cap)
	{
		n = v_read((t_handle)h, out.p + total, out.cap - total);
		if (n > 0 && (size_t)n <= out.cap - total)
			total += (size_t)n;
		else if (n > 0)
			n = E_IO;
	}
	if (n > 0 && v_read((t_handle)h, &extra, 1) != 0)
		n = E_OVERFLOW;
	v_close((t_handle)h);
	if (n < 0)
		return (n);
	return ((int64_t)total);
}

int64_t	apkglue_fs_read(void *ctx, const char *path, t_text out)
{
	char	old[APKGLUE_PATH_MAX];
	int64_t	n;

	(void)ctx;
	n = read_into(path, out);
	if (n != E_NOENT || apkglue_old_path(old, path) < 0)
		return (n);
	return (read_into(old, out));
}

static int	write_all(t_handle h, t_span data)
{
	size_t	done;
	size_t	step;
	int64_t	n;

	done = 0;
	while (done < data.len)
	{
		step = data.len - done;
		if (step > APKGLUE_CHUNK)
			step = APKGLUE_CHUNK;
		n = v_write(h, data.p + done, step);
		if (n < 0)
			return ((int)n);
		if (n == 0 || (size_t)n > step)
			return (E_IO);
		done += (size_t)n;
	}
	return (v_fsync(h));
}

int	apkglue_fs_write_new(void *ctx, const char *path, t_span data)
{
	int64_t	h;
	int		r;
	int		c;

	(void)ctx;
	h = v_open(path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
	if (h < 0)
		return ((int)h);
	r = write_all((t_handle)h, data);
	c = v_close((t_handle)h);
	if (r == 0 && c < 0)
		r = c;
	return (r);
}
