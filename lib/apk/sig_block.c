#include "apk_int.h"

static int	sig_eocd(const t_apk *a, size_t *eocd)
{
	t_span	f;
	size_t	pos;
	size_t	i;

	f = a->file;
	pos = (size_t)a->zip.cd_off + a->zip.cd_size;
	if (pos > f.len || f.len - pos < APK_EOCD_LEN
		|| apk_rd32(f.p + pos) != APK_EOCD_SIG)
		return (E_ACCES);
	if (apk_rd32(f.p + pos + 12) != a->zip.cd_size
		|| apk_rd32(f.p + pos + 16) != a->zip.cd_off
		|| (size_t)f.p[pos + 20] + ((size_t)f.p[pos + 21] << 8)
		!= f.len - pos - APK_EOCD_LEN)
		return (E_ACCES);
	i = pos + APK_EOCD_LEN;
	while (i + APK_MAGIC_LEN <= f.len)
	{
		if (memcmp(f.p + i, APK_MAGIC, APK_MAGIC_LEN) == 0)
			return (E_ACCES);
		i++;
	}
	*eocd = pos;
	return (0);
}

static int	sig_pairs(t_span in, t_span *v2)
{
	uint64_t	n;
	uint32_t	found;

	found = 0;
	while (in.len >= 12)
	{
		n = apk_rd64(in.p);
		if (n < 4 || n > in.len - 8)
			return (E_ACCES);
		if (apk_rd32(in.p + 8) == APK_SIG_V2_ID)
		{
			*v2 = (t_span){in.p + 12, (size_t)n - 4};
			found++;
		}
		in = (t_span){in.p + 8 + (size_t)n, in.len - 8 - (size_t)n};
	}
	if (in.len != 0 || found != 1)
		return (E_ACCES);
	return (0);
}

static int	sig_entries_before(const t_apk *a, size_t limit)
{
	t_zipent	e;
	uint32_t	i;

	i = 0;
	while (i < a->zip.count)
	{
		if (zip_entry(&a->zip, i, &e) < 0)
			return (E_ACCES);
		if (e.lfh_off >= limit || e.data_off > limit
			|| e.csize > limit - e.data_off)
			return (E_ACCES);
		i++;
	}
	return (0);
}

int	sig_locate(const t_apk *a, t_sigblock *b)
{
	t_span		f;
	size_t		cd;
	uint64_t	size;

	f = a->file;
	cd = a->zip.cd_off;
	if (sig_eocd(a, &b->eocd) < 0 || cd > f.len || cd < 32
		|| memcmp(f.p + cd - APK_MAGIC_LEN, APK_MAGIC, APK_MAGIC_LEN) != 0)
		return (E_ACCES);
	size = apk_rd64(f.p + cd - 24);
	if (size < 24 || size > cd - 8)
		return (E_ACCES);
	b->off = cd - 8 - (size_t)size;
	if (apk_rd64(f.p + b->off) != size)
		return (E_ACCES);
	if (sig_pairs((t_span){f.p + b->off + 8, (size_t)size - 24}, &b->v2) < 0)
		return (E_ACCES);
	return (sig_entries_before(a, b->off));
}
