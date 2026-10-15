#include "velum/libk.h"
#include "cpu_int.h"

static void	brand_trim(char *s)
{
	size_t	start;
	size_t	len;

	start = 0;
	while (s[start] == ' ')
		start++;
	len = strlen(s + start);
	memmove(s, s + start, len + 1);
	while (len > 0 && s[len - 1] == ' ')
	{
		s[len - 1] = '\0';
		len--;
	}
}

void	cpuid_brand(const t_cpuid_raw *raw, char out[49])
{
	memcpy(out, raw->r[CPUID_E2], 16);
	memcpy(out + 16, raw->r[CPUID_E3], 16);
	memcpy(out + 32, raw->r[CPUID_E4], 16);
	out[48] = '\0';
	brand_trim(out);
}
