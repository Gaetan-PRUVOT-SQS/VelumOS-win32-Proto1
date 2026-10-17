#include "fuzz.h"
#include "fx.h"

#define AXML_FUZZ_MAX 0x10000u

int	LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
	t_span	s;

	if (size > AXML_FUZZ_MAX)
		return (0);
	s.p = data;
	s.len = size;
	FUZZ_REQUIRE(fx_walk(FX_AXML, s) <= 0);
	FUZZ_REQUIRE(fx_walk(FX_POOL, s) <= 0);
	FUZZ_REQUIRE(fx_walk(FX_ARSC, s) <= 0);
	return (0);
}
