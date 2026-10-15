#include <string.h>
#include "cpio.h"
#include "fuzz.h"
#include "velum/err.h"
#include "velum/heap.h"

#define CPIO_FUZZ_MAX 0x10000u
#define CPIO_FUZZ_NAME_MAX 512

static void	check_order(const t_initrd *rd)
{
	uint32_t	i;

	i = 1;
	while (i < rd->n)
	{
		FUZZ_REQUIRE(cpio_cmp(rd->ents[i - 1].name, rd->ents[i - 1].nlen,
				rd->ents[i].name, rd->ents[i].nlen) <= 0);
		i++;
	}
}

static void	check_entry(t_initrd *rd, const t_cpent *e)
{
	char		key[CPIO_FUZZ_NAME_MAX];
	uint8_t		out[256];
	t_vnode		n;
	t_vio		io;
	int64_t		got;

	FUZZ_REQUIRE(e->nlen > 0 && e->nlen < CPIO_FUZZ_NAME_MAX);
	memcpy(key, e->name, e->nlen);
	key[e->nlen] = '\0';
	FUZZ_REQUIRE(rd_lookup(rd, key, &n) == 0);
	if (!(n.mode & S_TYPE_REG))
		return ;
	io.dst = out;
	io.src = NULL;
	io.len = sizeof(out);
	io.off = 0;
	got = rd_read(rd, &n, &io);
	FUZZ_REQUIRE(got >= 0 && (uint64_t)got <= sizeof(out));
	FUZZ_REQUIRE((uint64_t)got <= n.size);
}

static void	walk_dir(t_initrd *rd, t_vnode *d)
{
	t_dirent	o;
	uint64_t	ck;
	uint32_t	count;

	ck = 0;
	count = 0;
	while (rd_readdir(rd, d, &ck, &o) == 1)
	{
		FUZZ_REQUIRE(++count <= rd->n);
		FUZZ_REQUIRE(o.namelen > 0 && o.name[o.namelen] == '\0');
	}
}

static void	check_index(t_initrd *rd)
{
	t_vnode		root;
	uint32_t	i;

	check_order(rd);
	i = 0;
	while (i < rd->n)
		check_entry(rd, &rd->ents[i++]);
	FUZZ_REQUIRE(rd_lookup(rd, "", &root) == 0);
	walk_dir(rd, &root);
}

int	LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
	t_initrd	rd;
	int			rc;

	if (size > CPIO_FUZZ_MAX)
		return (0);
	fuzz_heap_fail_after(0);
	rc = cpio_index(data, size, &rd);
	FUZZ_REQUIRE(rc <= 0 && fuzz_heap_live() == 0);
	fuzz_heap_fail_after(-1);
	rc = cpio_index(data, size, &rd);
	if (rc == 0)
	{
		check_index(&rd);
		kfree(rd.ents);
	}
	FUZZ_REQUIRE(fuzz_heap_live() == 0);
	return (0);
}
