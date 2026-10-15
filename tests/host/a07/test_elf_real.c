#include <stdio.h>
#include <stdlib.h>
#include "a07_fake.h"
#include "harness.h"

static const char	*g_real[] = {"init", "ring3test", "vtest", NULL};

static uint8_t	*real_load(const char *dir, const char *name, uint64_t *size)
{
	char	path[512];
	FILE	*f;
	uint8_t	*buf;
	long	n;

	snprintf(path, sizeof(path), "%s/%s", dir, name);
	f = fopen(path, "rb");
	if (!f)
		return (NULL);
	buf = NULL;
	if (fseek(f, 0, SEEK_END) == 0)
	{
		n = ftell(f);
		buf = malloc((size_t)n + 1);
		*size = (uint64_t)n;
		rewind(f);
		if (buf && fread(buf, 1, (size_t)n, f) != (size_t)n)
			*size = 0;
	}
	fclose(f);
	return (buf);
}

static void	real_files(void)
{
	const char	*dir;
	uint8_t		*img;
	uint64_t	size;
	t_elfinfo	in;
	int			i;

	dir = getenv("A07_ELF_DIR");
	if (!dir)
	{
		printf("a07/elf_reels : A07_ELF_DIR absent, rien a lire\n");
		return ;
	}
	i = 0;
	while (g_real[i])
	{
		img = real_load(dir, g_real[i], &size);
		h_true(img != NULL, g_real[i]);
		if (img)
			h_eq_i64(g_real[i], elf_validate(img, size, &in), 0);
		if (img)
			h_true(in.type == ELF_ET_DYN && in.nseg >= 2, "PIE en segments");
		free(img);
		i++;
	}
}

int	main(void)
{
	h_begin("a07/elf_reels");
	h_run("binaires de la chaine", real_files);
	return (h_end());
}
