#include <stdlib.h>
#include <string.h>
#include "a07_fake.h"

void	gen_get_eh(const t_gen *g, t_elf64_ehdr *eh)
{
	memcpy(eh, g->buf, sizeof(*eh));
}

void	gen_set_eh(t_gen *g, const t_elf64_ehdr *eh)
{
	memcpy(g->buf, eh, sizeof(*eh));
}

void	gen_get_ph(const t_gen *g, int i, t_elf64_phdr *ph)
{
	memcpy(ph, g->buf + sizeof(t_elf64_ehdr) + i * sizeof(*ph), sizeof(*ph));
}

void	gen_set_ph(t_gen *g, int i, const t_elf64_phdr *ph)
{
	memcpy(g->buf + sizeof(t_elf64_ehdr) + i * sizeof(*ph), ph, sizeof(*ph));
}

int	gen_check(t_gen *g, t_elfinfo *out)
{
	uint8_t	*exact;
	size_t	alloc;
	int		rc;

	alloc = g->size;
	if (alloc == 0)
		alloc = 1;
	exact = malloc(alloc);
	if (!exact)
		abort();
	memcpy(exact, g->buf, g->size);
	rc = elf_validate(exact, g->size, out);
	free(exact);
	return (rc);
}
