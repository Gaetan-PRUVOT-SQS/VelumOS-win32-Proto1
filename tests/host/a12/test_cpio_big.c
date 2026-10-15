#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "harness.h"
#include "fake.h"
#include "cpio.h"
#include "velum/err.h"
#include "velum/heap.h"

static void	volume_10000(void)
{
	t_vbuf		b;
	t_initrd	rd;
	t_vnode		n;
	char		name[32];
	uint32_t	i;

	b.cap = 2000000;
	b.len = 0;
	b.p = malloc(b.cap);
	i = 0;
	while (i < 10000)
	{
		snprintf(name, sizeof(name), "d%u/f%05u", i % 37, i);
		vst_cpio_add(&b, name, 0100644, "x");
		i++;
	}
	vst_cpio_add(&b, "TRAILER!!!", 0, NULL);
	h_eq_i64("10 000 entrées", cpio_index(b.p, b.len, &rd), 0);
	h_eq_u64("toutes gardées", rd.n, 10000);
	h_true(rd_lookup(&rd, "d5/f09995", &n) == 0 && n.size == 1, "recherche");
	h_true(rd_lookup(&rd, "d36", &n) == 0 && (n.mode & S_TYPE_DIR),
		"dossier implicite parmi 10 000");
	kfree(rd.ents);
	free(b.p);
}

static int	ents_inside(const t_initrd *rd, const uint8_t *c, size_t len)
{
	const t_cpent	*e;
	uint32_t		i;

	i = 0;
	while (i < rd->n)
	{
		e = &rd->ents[i];
		if (e->data < c || e->size > len || e->data + e->size > c + len
			|| (const uint8_t *)e->name < c
			|| (const uint8_t *)e->name + e->nlen > c + len)
			return (0);
		i++;
	}
	return (1);
}

static void	fuzz_octets(void)
{
	uint8_t		raw[4096];
	t_vbuf		b;
	t_initrd	rd;
	uint64_t	seed;
	uint8_t		*c;

	b.p = raw;
	b.len = 0;
	b.cap = sizeof(raw);
	fakecpio_valid(&b);
	seed = fake_seed();
	while (b.cap < 4096 + 5000)
	{
		c = fake_dup(raw, b.len);
		c[fake_rand(&seed) % b.len] = (uint8_t)fake_rand(&seed);
		c[fake_rand(&seed) % b.len] = (uint8_t)fake_rand(&seed);
		if (cpio_index(c, b.len, &rd) == 0)
		{
			h_true(ents_inside(&rd, c, b.len), "fuzz : entrées dans le module");
			kfree(rd.ents);
		}
		free(c);
		b.cap++;
	}
}

int	main(void)
{
	h_begin("a12/cpio_volume");
	h_run("volume 10 000 entrées", volume_10000);
	h_run("fuzz octets à graine", fuzz_octets);
	return (h_end());
}
