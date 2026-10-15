#include <stdlib.h>
#include "velum/err.h"
#include "velum/libk.h"
#include "velum/vfile.h"
#include "velum/vobj.h"
#include "apkglue.h"

static int64_t	read_all(t_handle h, uint8_t *buf, size_t size)
{
	size_t	total;
	int64_t	n;

	total = 0;
	n = 1;
	while (n > 0 && total < size)
	{
		n = v_read(h, buf + total, size - total);
		if (n > 0 && (size_t)n > size - total)
			return (E_IO);
		if (n > 0)
			total += (size_t)n;
	}
	if (n < 0)
		return (n);
	if (total != size)
		return (E_IO);
	return ((int64_t)total);
}

int64_t	apkglue_read_file(const char *path, uint8_t **out)
{
	t_vstat	st;
	int64_t	h;
	int64_t	n;
	uint8_t	*buf;

	*out = NULL;
	h = v_open(path, O_RDONLY, 0);
	if (h < 0)
		return (h);
	buf = NULL;
	n = v_fstat((t_handle)h, &st);
	if (n == 0 && st.size > APKGLUE_FILE_MAX)
		n = E_RANGE;
	if (n == 0)
		buf = malloc((size_t)st.size + 1);
	if (n == 0 && !buf)
		n = E_NOMEM;
	if (n == 0)
		n = read_all((t_handle)h, buf, (size_t)st.size);
	v_close((t_handle)h);
	if (n < 0)
		free(buf);
	else
		*out = buf;
	return (n);
}

size_t	apkglue_label(char *dst, size_t cap, const char *src)
{
	size_t	n;

	if (cap == 0)
		return (0);
	n = 0;
	while (n < cap - 1 && src[n])
		n++;
	if (src[n])
	{
		while (n > 0 && ((unsigned char)src[n] & 0xc0) == 0x80)
			n--;
	}
	memcpy(dst, src, n);
	dst[n] = '\0';
	return (n);
}
