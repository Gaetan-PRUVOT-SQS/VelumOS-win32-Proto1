#include "velum/err.h"
#include "velum/libk.h"
#include "../common/utf8.h"
#include "console.h"

static int	scalar_ok(const unsigned char *s, size_t n)
{
	uint32_t	cp;

	if (n == 1)
		return (s[0] >= 0x20 && s[0] != 0x7f);
	if (n != 2)
		return (1);
	cp = ((uint32_t)(s[0] & 0x1f) << 6) | (s[1] & 0x3f);
	return (cp >= 0xa0);
}

static int	bytes_ok(const char *s, size_t n)
{
	size_t	i;
	size_t	k;

	if (!s)
		return (n == 0);
	i = 0;
	while (i < n)
	{
		k = utf8_seq_len((const uint8_t *)s + i, n - i);
		if (k == 0 || !scalar_ok((const unsigned char *)s + i, k))
			return (0);
		i += k;
	}
	return (1);
}

static int	push_part(char *out, size_t size, size_t *len, const char *part,
				size_t n)
{
	if (n >= size || *len >= size - 1 - n)
		return (E_RANGE);
	out[*len] = '/';
	memcpy(out + *len + 1, part, n);
	*len += 1 + n;
	out[*len] = '\0';
	return (0);
}

static void	pop_part(char *out, size_t *len)
{
	while (*len > 0 && out[*len - 1] != '/')
		(*len)--;
	if (*len > 0)
		(*len)--;
	out[*len] = '\0';
}

static int	apply_part(char *out, size_t size, size_t *len, const char *part,
				size_t n)
{
	if (n > 0 && !bytes_ok(part, n))
		return (E_INVAL);
	if (n == 0 || (n == 1 && part[0] == '.'))
		return (0);
	if (n == 2 && part[0] == '.' && part[1] == '.')
	{
		pop_part(out, len);
		return (0);
	}
	return (push_part(out, size, len, part, n));
}

static int	walk(const char *p, char *out, size_t size, size_t *len)
{
	const char	*end;
	int			rc;

	while (*p)
	{
		end = p;
		while (*end && *end != '/')
			end++;
		rc = apply_part(out, size, len, p, (size_t)(end - p));
		if (rc < 0)
			return (rc);
		p = end;
		if (*p == '/')
			p++;
	}
	return (0);
}

int	con_path_join(const char *cwd, const char *arg, char *out, size_t size)
{
	size_t	len;
	int		rc;

	if (size < 2 || !arg || !cwd)
		return (E_INVAL);
	len = 0;
	out[0] = '\0';
	rc = 0;
	if (arg[0] != '/')
		rc = walk(cwd, out, size, &len);
	if (rc == 0)
		rc = walk(arg, out, size, &len);
	if (rc < 0)
		return (rc);
	if (len == 0)
		strlcpy(out, "/", size);
	if (strlen(out) >= VFS_PATH_MAX)
		return (E_RANGE);
	return (0);
}

int	con_path_in_bin(const char *path)
{
	size_t		n;
	const char	*base;

	if (!path)
		return (0);
	n = sizeof(CON_BIN_DIR) - 1;
	if (strncmp(path, CON_BIN_DIR, n) != 0 || path[n] != '/')
		return (0);
	base = path + n + 1;
	return (base[0] != '\0' && strchr(base, '/') == NULL);
}

static int	under_system(const char *path)
{
	size_t	n;

	n = sizeof(CON_SYS_DIR) - 1;
	if (strncmp(path, CON_SYS_DIR, n) != 0)
		return (0);
	return (path[n] == '\0' || path[n] == '/');
}

static int	path_like(const char *s)
{
	if (s[0] == '.' && (s[1] == '\0' || (s[1] == '.' && s[2] == '\0')))
		return (1);
	return (strchr(s, '/') != NULL);
}

int	con_confine_arg(const char *cwd, const char *arg, char *out, size_t size)
{
	int	rc;

	if (!arg || !out || size < 2)
		return (E_INVAL);
	out[0] = '\0';
	if (!bytes_ok(arg, strlen(arg)))
		return (E_INVAL);
	if (!path_like(arg))
	{
		if (strlcpy(out, arg, size) >= size)
		{
			out[0] = '\0';
			return (E_RANGE);
		}
		return (0);
	}
	rc = con_path_join(cwd, arg, out, size);
	if (rc < 0)
		return (rc);
	if (!under_system(out))
	{
		out[0] = '\0';
		return (E_ACCES);
	}
	return (0);
}

const char	*con_err_text(int err)
{
	if (err == E_NOENT)
		return ("introuvable");
	if (err == E_NOTDIR)
		return ("n'est pas un dossier");
	if (err == E_ISDIR)
		return ("est un dossier");
	if (err == E_ACCES || err == E_PERM)
		return ("accès refusé");
	if (err == E_NOMEM)
		return ("mémoire insuffisante");
	if (err == E_RANGE)
		return ("trop long");
	if (err == E_INVAL)
		return ("argument invalide");
	return ("erreur système");
}
