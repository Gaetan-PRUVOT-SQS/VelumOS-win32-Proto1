#include "apk_int.h"

static int	refuse(t_apk *a, int reason, int err)
{
	a->reason = reason;
	a->file = (t_span){NULL, 0};
	return (err);
}

static int	is_native(const t_zipent *e)
{
	if (e->name_len < 7)
		return (0);
	return (memcmp(e->name, "lib/", 4) == 0
		&& memcmp(e->name + e->name_len - 3, ".so", 3) == 0);
}

static int	is_extra_dex(const t_zipent *e)
{
	uint32_t	i;

	if (e->name_len < 12 || memcmp(e->name, "classes", 7) != 0
		|| memcmp(e->name + e->name_len - 4, ".dex", 4) != 0)
		return (0);
	i = 7;
	while (i < e->name_len - 4)
	{
		if (e->name[i] < '0' || e->name[i] > '9')
			return (0);
		i++;
	}
	return (1);
}

static int	scan(t_apk *a)
{
	t_zipent	e;
	uint32_t	i;

	i = 0;
	while (i < a->zip.count)
	{
		if (zip_entry(&a->zip, i, &e) < 0)
			return (refuse(a, APKR_ARCHIVE, E_INVAL));
		if (zip_name_safe(e.name, e.name_len) != 1)
			return (refuse(a, APKR_NOM, E_INVAL));
		if (is_native(&e))
			return (refuse(a, APKR_NATIF, E_NOTSUP));
		if (is_extra_dex(&e))
			return (refuse(a, APKR_MULTIDEX, E_NOTSUP));
		i++;
	}
	return (0);
}

int	apk_open(t_apk *a, t_span file)
{
	int	r;

	if (a == NULL)
		return (E_INVAL);
	memset(a, 0, sizeof(*a));
	if (file.p == NULL)
		return (refuse(a, APKR_ARCHIVE, E_INVAL));
	if (file.len > APK_FILE_MAX)
		return (refuse(a, APKR_TAILLE, E_RANGE));
	if (zip_open(&a->zip, file) < 0)
		return (refuse(a, APKR_ARCHIVE, E_INVAL));
	a->file = file;
	r = scan(a);
	if (r < 0)
		return (r);
	if (zip_find(&a->zip, "AndroidManifest.xml", &a->manifest) < 0
		|| zip_find(&a->zip, "classes.dex", &a->dex) < 0)
		return (refuse(a, APKR_MANQUE, E_NOENT));
	if (a->manifest.usize > APK_ENTRY_MAX || a->dex.usize > APK_ENTRY_MAX)
		return (refuse(a, APKR_TAILLE, E_RANGE));
	a->reason = APKR_OK;
	return (0);
}
