#include "velum/err.h"
#include "velum/libk.h"
#include "velum/vfile.h"
#include "fake_vfile.h"

#define FV_FILES 4
#define FV_NAME 96
#define FV_DATA 640
#define FV_STEP 5

typedef struct s_fvfile
{
	int		used;
	int		open;
	int		writable;
	char	name[FV_NAME];
	char	data[FV_DATA];
	size_t	len;
	size_t	pos;
}	t_fvfile;

static t_fvfile	g_f[FV_FILES];
static int		g_write_left = -1;
static int		g_read_left = -1;
static int		g_rename_fail;
static int		g_fsync_fail;
static int		g_open_fail;

static int	find_name(const char *path)
{
	int	i;

	i = 0;
	while (i < FV_FILES)
	{
		if (g_f[i].used && strcmp(g_f[i].name, path) == 0)
			return (i);
		i++;
	}
	return (-1);
}

static int	alloc_slot(void)
{
	int	i;

	i = 0;
	while (i < FV_FILES)
	{
		if (!g_f[i].used)
			return (i);
		i++;
	}
	return (-1);
}

static int	slot_of(t_handle file)
{
	if (file == 0 || file > FV_FILES)
		return (-1);
	if (!g_f[file - 1].used || !g_f[file - 1].open)
		return (-1);
	return ((int)file - 1);
}

static int64_t	take(int *budget, size_t want)
{
	size_t	n;

	if (want == 0)
		return (0);
	if (*budget == 0)
		return (E_IO);
	n = want;
	if (n > FV_STEP)
		n = FV_STEP;
	if (*budget > 0 && (size_t)*budget < n)
		n = (size_t)*budget;
	if (*budget > 0)
		*budget -= (int)n;
	return ((int64_t)n);
}

void	fv_reset(void)
{
	memset(g_f, 0, sizeof(g_f));
	g_write_left = -1;
	g_read_left = -1;
	g_rename_fail = 0;
	g_fsync_fail = 0;
	g_open_fail = 0;
}

void	fv_write_budget(int bytes)
{
	g_write_left = bytes;
}

void	fv_read_budget(int bytes)
{
	g_read_left = bytes;
}

void	fv_rename_fail(int on)
{
	g_rename_fail = on;
}

void	fv_fsync_fail(int on)
{
	g_fsync_fail = on;
}

void	fv_open_fail(int on)
{
	g_open_fail = on;
}

int	fv_nfiles(void)
{
	int	i;
	int	n;

	n = 0;
	i = 0;
	while (i < FV_FILES)
	{
		if (g_f[i].used)
			n++;
		i++;
	}
	return (n);
}

int	fv_plant(const char *path, const void *data, size_t len)
{
	int	i;

	if (!path || path[0] == '\0' || strlen(path) >= FV_NAME)
		return (E_INVAL);
	if (len > FV_DATA || (len > 0 && data == NULL))
		return (E_INVAL);
	i = find_name(path);
	if (i < 0)
		i = alloc_slot();
	if (i < 0)
		return (E_NOSPC);
	memset(&g_f[i], 0, sizeof(g_f[i]));
	strlcpy(g_f[i].name, path, sizeof(g_f[i].name));
	if (len > 0)
		memcpy(g_f[i].data, data, len);
	g_f[i].len = len;
	g_f[i].used = 1;
	return (0);
}

int	fv_slurp(const char *path, char *out, size_t max)
{
	int	i;

	if (!path || !out)
		return (E_INVAL);
	i = find_name(path);
	if (i < 0)
		return (E_NOENT);
	if (g_f[i].len > max)
		return (E_RANGE);
	if (g_f[i].len > 0)
		memcpy(out, g_f[i].data, g_f[i].len);
	return ((int)g_f[i].len);
}

int64_t	v_open(const char *path, uint32_t flags, uint32_t mode)
{
	int	i;
	int	writing;

	(void)mode;
	if (!path)
		return (E_FAULT);
	if (path[0] == '\0' || strlen(path) >= FV_NAME)
		return (E_INVAL);
	if (g_open_fail)
		return (E_IO);
	writing = (flags & (O_WRONLY | O_RDWR)) != 0;
	i = find_name(path);
	if (i < 0)
	{
		if ((flags & O_CREAT) == 0)
			return (E_NOENT);
		i = alloc_slot();
		if (i < 0)
			return (E_NOSPC);
		memset(&g_f[i], 0, sizeof(g_f[i]));
		strlcpy(g_f[i].name, path, sizeof(g_f[i].name));
		g_f[i].used = 1;
	}
	else if (g_f[i].open)
		return (E_BUSY);
	if (flags & O_TRUNC)
		g_f[i].len = 0;
	g_f[i].pos = 0;
	g_f[i].open = 1;
	g_f[i].writable = writing;
	return (i + 1);
}

int64_t	v_read(t_handle file, void *buf, size_t len)
{
	int		i;
	int64_t	n;
	size_t	left;

	i = slot_of(file);
	if (i < 0 || g_f[i].writable || buf == NULL)
		return (E_BADF);
	left = g_f[i].len - g_f[i].pos;
	if (left == 0 || len == 0)
		return (0);
	if (len < left)
		left = len;
	n = take(&g_read_left, left);
	if (n <= 0)
		return (n);
	memcpy(buf, g_f[i].data + g_f[i].pos, (size_t)n);
	g_f[i].pos += (size_t)n;
	return (n);
}

int64_t	v_write(t_handle file, const void *buf, size_t len)
{
	int		i;
	int64_t	n;
	size_t	room;

	i = slot_of(file);
	if (i < 0 || !g_f[i].writable || (len > 0 && buf == NULL))
		return (E_BADF);
	if (len == 0)
		return (0);
	room = FV_DATA - g_f[i].len;
	if (room == 0)
		return (E_NOSPC);
	if (len < room)
		room = len;
	n = take(&g_write_left, room);
	if (n <= 0)
		return (n);
	memcpy(g_f[i].data + g_f[i].len, buf, (size_t)n);
	g_f[i].len += (size_t)n;
	return (n);
}

int	v_close(t_handle h)
{
	int	i;

	i = slot_of(h);
	if (i < 0)
		return (E_BADF);
	g_f[i].open = 0;
	return (0);
}

int	v_unlink(const char *path)
{
	int	i;

	if (!path)
		return (E_FAULT);
	i = find_name(path);
	if (i < 0)
		return (E_NOENT);
	memset(&g_f[i], 0, sizeof(g_f[i]));
	return (0);
}

int	v_rename(const char *old_path, const char *new_path)
{
	int	from;
	int	to;

	if (!old_path || !new_path)
		return (E_FAULT);
	if (g_rename_fail)
		return (E_IO);
	from = find_name(old_path);
	if (from < 0)
		return (E_NOENT);
	if (strcmp(old_path, new_path) == 0)
		return (0);
	if (strlen(new_path) >= FV_NAME)
		return (E_RANGE);
	to = find_name(new_path);
	if (to >= 0)
	{
		g_f[to].used = 0;
		g_f[to].open = 0;
	}
	strlcpy(g_f[from].name, new_path, sizeof(g_f[from].name));
	return (0);
}

int	v_fsync(t_handle file)
{
	int	i;

	i = slot_of(file);
	if (i < 0)
		return (E_BADF);
	if (g_fsync_fail)
		return (E_IO);
	return (0);
}
