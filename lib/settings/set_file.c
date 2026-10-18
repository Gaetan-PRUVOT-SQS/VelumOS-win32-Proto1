#include "velum/err.h"
#include "velum/libk.h"
#include "velum/settings.h"
#include "velum/vfile.h"
#include "velum/vobj.h"

#define SET_TMP_SUFFIX ".tmp"

int	set_load(const char *path, t_settings *s)
{
	char	buf[SET_FILE_MAX + 1];
	int64_t	h;
	int64_t	n;
	size_t	total;

	if (!s)
		return (E_INVAL);
	set_default(s);
	if (!path)
		return (E_INVAL);
	h = v_open(path, O_RDONLY, 0);
	if (h < 0)
		return ((int)h);
	total = 0;
	n = 1;
	while (n > 0 && total < sizeof(buf))
	{
		n = v_read((t_handle)h, buf + total, sizeof(buf) - total);
		if (n > 0)
			total += (size_t)n;
	}
	v_close((t_handle)h);
	if (n < 0)
		return ((int)n);
	return (set_parse(buf, total, s));
}

static int	write_all(t_handle h, const char *buf, size_t len)
{
	int64_t	n;
	size_t	done;

	done = 0;
	while (done < len)
	{
		n = v_write(h, buf + done, len - done);
		if (n <= 0)
			return (n < 0 ? (int)n : E_IO);
		done += (size_t)n;
	}
	return (0);
}

int	set_save(const char *path, const t_settings *s)
{
	char	text[SET_FILE_MAX];
	char	tmp[96];
	int64_t	h;
	int		len;
	int		r;

	if (!path)
		return (E_INVAL);
	len = set_format(s, text, sizeof(text));
	if (len < 0)
		return (len);
	if (strlen(path) + sizeof(SET_TMP_SUFFIX) > sizeof(tmp))
		return (E_RANGE);
	strlcpy(tmp, path, sizeof(tmp));
	strlcat(tmp, SET_TMP_SUFFIX, sizeof(tmp));
	h = v_open(tmp, O_WRONLY | O_CREAT | O_TRUNC, 0644);
	if (h < 0)
		return ((int)h);
	r = write_all((t_handle)h, text, (size_t)len);
	if (r == 0)
		r = v_fsync((t_handle)h);
	v_close((t_handle)h);
	if (r < 0)
	{
		v_unlink(tmp);
		return (r);
	}
	r = v_rename(tmp, path);
	if (r < 0)
		v_unlink(tmp);
	return (r);
}
