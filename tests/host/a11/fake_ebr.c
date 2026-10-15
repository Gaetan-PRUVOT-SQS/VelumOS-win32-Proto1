#include <string.h>
#include "fake.h"

void	fk_ebr_sector(uint8_t *s, const t_fkpart *log, const t_fkpart *next)
{
	memset(s, 0, 512);
	if (log)
		fk_mbr_entry(s, 0, 0x83, log);
	if (next)
		fk_mbr_entry(s, 1, 0x05, next);
	s[510] = 0x55;
	s[511] = 0xaa;
}

void	fk_ebr_chain(t_fkram *r, uint64_t ext, uint32_t n, uint32_t stride)
{
	t_fkpart	log;
	t_fkpart	next;
	uint8_t		*s;
	uint32_t	i;

	log.first = 1;
	log.last = stride - 1;
	if (n == 0)
		fk_ebr_sector(r->data + ext * 512, NULL, NULL);
	i = 0;
	while (i < n)
	{
		s = r->data + (ext + (uint64_t)i * stride) * 512;
		next.first = (uint64_t)(i + 1) * stride;
		next.last = next.first + stride - 1;
		fk_ebr_sector(s, &log, &next);
		if (i + 1 == n)
			fk_ebr_sector(s, &log, NULL);
		i++;
	}
}
