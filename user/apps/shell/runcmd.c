#include "velum/err.h"
#include "velum/libk.h"
#include "runcmd.h"

static int	name_char_ok(char c, size_t pos)
{
	if ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'z')
		|| (c >= 'A' && c <= 'Z'))
		return (1);
	return (pos > 0 && (c == '_' || c == '-' || c == '.'));
}

int	run_name_ok(const char *s, size_t n)
{
	size_t	i;

	if (n == 0 || n > RUN_NAME_MAX)
		return (0);
	i = 0;
	while (i < n)
	{
		if (!name_char_ok(s[i], i))
			return (0);
		if (s[i] == '.' && i + 1 < n && s[i + 1] == '.')
			return (0);
		i++;
	}
	return (1);
}

static void	trim(const char **s, size_t *n)
{
	while (*n && ((*s)[0] == ' ' || (*s)[0] == '\t'))
	{
		(*s)++;
		(*n)--;
	}
	while (*n && ((*s)[*n - 1] == ' ' || (*s)[*n - 1] == '\t'))
		(*n)--;
}

int	run_resolve(const char *input, size_t len, char *path, size_t size)
{
	size_t	dir;

	if (!input || !path || size == 0 || len > RUN_INPUT_MAX)
		return (E_INVAL);
	trim(&input, &len);
	dir = strlen(RUN_DIR);
	if (len > dir && strncmp(input, RUN_DIR, dir) == 0)
	{
		input += dir;
		len -= dir;
	}
	if (!run_name_ok(input, len))
		return (E_INVAL);
	if (dir + len + 1 > size)
		return (E_RANGE);
	memcpy(path, RUN_DIR, dir);
	memcpy(path + dir, input, len);
	path[dir + len] = '\0';
	return (0);
}

int	run_apk_path(const char *input, size_t len, char *path, size_t size)
{
	size_t	i;

	if (!input || !path || len > RUN_INPUT_MAX)
		return (E_INVAL);
	trim(&input, &len);
	if (len < 6 || len >= size || len >= RUN_APK_MAX || input[0] != '/'
		|| memcmp(input + len - 4, ".apk", 4) != 0)
		return (E_INVAL);
	i = 0;
	while (i < len)
	{
		if ((unsigned char)input[i] <= 0x20
			|| (unsigned char)input[i] >= 0x7f
			|| (input[i] == '.' && i + 1 < len && input[i + 1] == '.')
			|| (input[i] == '/' && i + 1 < len && input[i + 1] == '/'))
			return (E_INVAL);
		i++;
	}
	memcpy(path, input, len);
	path[len] = '\0';
	return (0);
}
