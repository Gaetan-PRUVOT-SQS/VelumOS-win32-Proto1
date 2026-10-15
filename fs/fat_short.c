#include "fat.h"
#include "velum/err.h"
#include "velum/libk.h"

static int	has_lower(const char *s)
{
	while (*s)
	{
		if (*s >= 'a' && *s <= 'z')
			return (1);
		s++;
	}
	return (0);
}

static int	sname_taken(t_fat *fs, uint32_t first, const uint8_t *raw11)
{
	uint8_t	tmp[FAT_DENT];
	char	s[13];
	t_fent	e;
	int		rc;

	memset(tmp, 0, sizeof(tmp));
	memcpy(tmp, raw11, 11);
	fat_sname_decode(tmp, s);
	rc = fdir_find(fs, first, s, &e);
	if (rc == E_NOENT)
		return (0);
	if (rc < 0)
		return (rc);
	return (1);
}

static int	pick_tail(t_fat *fs, uint32_t first, const char *name, t_fadd *a)
{
	uint8_t		basis[11];
	uint32_t	k;
	int			lossy;
	int			rc;

	fat_basis(name, basis, &lossy);
	if (!lossy && fat_fits_83(name))
	{
		memcpy(a->raw, basis, 11);
		rc = sname_taken(fs, first, basis);
		if (rc == 0 && !has_lower(name))
			a->nlfn = 0;
		if (rc <= 0)
			return (rc);
	}
	k = 1;
	while (k <= FAT_TAIL_TRIES)
	{
		fat_sname_try(name, basis, k, a->raw);
		rc = sname_taken(fs, first, a->raw);
		if (rc <= 0)
			return (rc);
		k++;
	}
	return (E_EXIST);
}

int	fat_pick_short(t_fat *fs, uint32_t first, const char *name, t_fadd *a)
{
	t_fent	e;
	int		rc;

	rc = fdir_find(fs, first, name, &e);
	if (rc == 0)
		return (E_EXIST);
	if (rc != E_NOENT)
		return (rc);
	rc = fat_utf8_to16(name, fs->u16, VFS_NAME_MAX);
	if (rc <= 0)
		return (E_INVAL);
	a->u = fs->u16;
	a->ulen = (uint32_t)rc;
	a->nlfn = (a->ulen + FAT_LFN_CHARS - 1) / FAT_LFN_CHARS;
	return (pick_tail(fs, first, name, a));
}
