#include <stdlib.h>
#include <string.h>
#include "fuzz.h"
#include "velum/apk/zip.h"
#include "velum/err.h"
#ifdef FUZZ_WITH_ZLIB
# include <zlib.h>
#endif

#define INFLATE_FUZZ_MAX 16384u
#define INFLATE_OUT_CAP 65536u

static uint8_t	*alloc_exact(size_t n)
{
	uint8_t	*p;

	p = malloc(n + (n == 0));
	if (p == NULL)
		abort();
	return (p);
}

static size_t	small_cap(const uint8_t *data, size_t size)
{
	size_t	cap;

	cap = size * 7;
	if (size > 0)
		cap += data[0];
	return (cap & 0x3ff);
}

static void	check_cap_monotone(t_span in, const uint8_t *ref, int64_t n,
		size_t cap)
{
	uint8_t	*out;
	int64_t	m;

	out = alloc_exact(cap);
	m = inflate_raw(in, out, cap);
	if ((uint64_t)n <= cap)
	{
		FUZZ_REQUIRE(m == n);
		FUZZ_REQUIRE(memcmp(out, ref, (size_t)n) == 0);
	}
	else
		FUZZ_REQUIRE(m == E_OVERFLOW);
	free(out);
}

#ifdef FUZZ_WITH_ZLIB
static void	check_against_zlib(const uint8_t *data, size_t size,
		const uint8_t *ref, int64_t n)
{
	z_stream	zs;
	uint8_t		*out;
	int			rc;

	memset(&zs, 0, sizeof(zs));
	FUZZ_REQUIRE(inflateInit2(&zs, -15) == Z_OK);
	out = alloc_exact(INFLATE_OUT_CAP);
	zs.next_in = (Bytef *)(uintptr_t)data;
	zs.avail_in = (uInt)size;
	zs.next_out = out;
	zs.avail_out = INFLATE_OUT_CAP;
	rc = inflate(&zs, Z_FINISH);
	FUZZ_REQUIRE(rc == Z_STREAM_END);
	FUZZ_REQUIRE(zs.total_out == (uLong)n);
	FUZZ_REQUIRE(memcmp(out, ref, (size_t)n) == 0);
	inflateEnd(&zs);
	free(out);
}
#endif

int	LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
	t_span	in;
	uint8_t	*out;
	int64_t	n;

	if (size > INFLATE_FUZZ_MAX)
		return (0);
	in.p = data;
	in.len = size;
	out = alloc_exact(INFLATE_OUT_CAP);
	n = inflate_raw(in, out, INFLATE_OUT_CAP);
	FUZZ_REQUIRE(n <= (int64_t)INFLATE_OUT_CAP);
	if (n >= 0)
	{
		check_cap_monotone(in, out, n, small_cap(data, size));
		check_cap_monotone(in, out, n, (size_t)n);
#ifdef FUZZ_WITH_ZLIB
		check_against_zlib(data, size, out, n);
#endif
	}
	free(out);
	return (0);
}
