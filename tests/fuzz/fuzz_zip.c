#include <stdlib.h>
#include <string.h>
#include "fuzz.h"
#include "velum/apk/zip.h"
#include "velum/err.h"

#define ZIP_FUZZ_MAX 0x10000u
#define ZIP_FUZZ_ENTRIES 48u
#define ZIP_FUZZ_UNPACK_MAX 0x40000u

static void	check_extract(const t_zip *z, const t_zipent *e)
{
	uint8_t	*out;
	t_span	in;
	int64_t	n;

	if (e->usize > ZIP_FUZZ_UNPACK_MAX)
		return ;
	out = malloc(e->usize + (e->usize == 0));
	if (out == NULL)
		abort();
	n = zip_extract(z, e, out, e->usize);
	FUZZ_REQUIRE(n == E_INVAL || n == E_NOTSUP || n == (int64_t)e->usize);
	if (e->method == 8)
	{
		in.p = z->file.p + e->data_off;
		in.len = e->csize;
		n = inflate_raw(in, out, e->usize);
		FUZZ_REQUIRE(n <= (int64_t)e->usize);
	}
	free(out);
}

static void	check_entry(const t_zip *z, uint32_t i)
{
	t_zipent	e;
	t_zipent	again;
	char		name[ZIP_NAME_MAX + 1];

	FUZZ_REQUIRE(zip_entry(z, i, &e) == 0);
	FUZZ_REQUIRE(e.name_len > 0 && e.name_len <= ZIP_NAME_MAX);
	FUZZ_REQUIRE(e.method == 0 || e.method == 8);
	FUZZ_REQUIRE((uint64_t)e.lfh_off + 30 + e.name_len <= z->cd_off);
	FUZZ_REQUIRE(e.data_off >= e.lfh_off + 30 + e.name_len);
	FUZZ_REQUIRE((uint64_t)e.data_off + e.csize <= z->cd_off);
	zip_name_safe(e.name, e.name_len);
	memcpy(name, e.name, e.name_len);
	name[e.name_len] = '\0';
	if (strlen(name) == e.name_len)
	{
		FUZZ_REQUIRE(zip_find(z, name, &again) == 0);
		FUZZ_REQUIRE(again.lfh_off == e.lfh_off);
	}
	check_extract(z, &e);
}

int	LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
	t_zip		z;
	t_span		file;
	uint32_t	i;
	int			rc;

	if (size > ZIP_FUZZ_MAX)
		return (0);
	file.p = data;
	file.len = size;
	rc = zip_open(&z, file);
	if (rc < 0)
	{
		FUZZ_REQUIRE(z.count == 0);
		return (0);
	}
	FUZZ_REQUIRE(z.count <= ZIP_MAX_ENTRIES);
	FUZZ_REQUIRE((uint64_t)z.cd_off + z.cd_size <= size);
	i = 0;
	while (i < z.count && i < ZIP_FUZZ_ENTRIES)
		check_entry(&z, i++);
	return (0);
}
