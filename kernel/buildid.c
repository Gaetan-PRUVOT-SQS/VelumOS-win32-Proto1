#include <stdint.h>
#include "velum/boot.h"

extern const uint8_t	g_build_id_note[];

const char	*boot_build_id(void)
{
	static char		hex[41];
	const uint8_t	*desc;
	uint32_t		len;
	uint32_t		i;

	if (hex[0])
		return (hex);
	len = *(const uint32_t *)(g_build_id_note + 4);
	desc = g_build_id_note + 12 + 4;
	if (len > 20)
		len = 20;
	i = 0;
	while (i < len)
	{
		hex[i * 2] = "0123456789abcdef"[desc[i] >> 4];
		hex[i * 2 + 1] = "0123456789abcdef"[desc[i] & 15];
		i++;
	}
	hex[len * 2] = '\0';
	return (hex);
}
