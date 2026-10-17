#include <stdlib.h>
#include <string.h>
#include "acpi_int.h"
#include "fuzz.h"
#include "velum/err.h"

#define ACPI_FUZZ_BASE 0xe0000ull
#define ACPI_FUZZ_MAX 0x4000u
#define ACPI_FUZZ_MAPS 512u

typedef struct s_acpimap
{
	void		*p;
	uint64_t	len;
}	t_acpimap;

typedef struct s_acpiwin
{
	const uint8_t	*raw;
	uint64_t		size;
	t_acpimap		maps[ACPI_FUZZ_MAPS];
}	t_acpiwin;

static t_acpiwin	g_win;

static const void	*win_map(uint64_t phys, uint64_t len, void *ctx)
{
	uint32_t	k;
	uint8_t		*copy;

	(void)ctx;
	if (phys < ACPI_FUZZ_BASE || len == 0
		|| phys - ACPI_FUZZ_BASE > g_win.size
		|| len > g_win.size - (phys - ACPI_FUZZ_BASE))
		return (NULL);
	k = 0;
	while (k < ACPI_FUZZ_MAPS && g_win.maps[k].p != NULL)
		k++;
	if (k == ACPI_FUZZ_MAPS)
		return (NULL);
	copy = malloc(len);
	if (copy == NULL)
		abort();
	memcpy(copy, g_win.raw + (phys - ACPI_FUZZ_BASE), len);
	g_win.maps[k].p = copy;
	g_win.maps[k].len = len;
	return (copy);
}

static void	win_unmap(const void *v, uint64_t len, void *ctx)
{
	uint32_t	k;

	(void)ctx;
	k = 0;
	while (k < ACPI_FUZZ_MAPS)
	{
		if (g_win.maps[k].p == v)
		{
			FUZZ_REQUIRE(g_win.maps[k].len == len);
			free(g_win.maps[k].p);
			g_win.maps[k].p = NULL;
			return ;
		}
		k++;
	}
	FUZZ_REQUIRE(v == NULL);
}

static void	win_release(void)
{
	uint32_t	k;

	k = 0;
	while (k < ACPI_FUZZ_MAPS)
	{
		free(g_win.maps[k].p);
		g_win.maps[k].p = NULL;
		k++;
	}
}

static void	fix_sum(uint8_t *p, uint64_t len, uint64_t at)
{
	p[at] = 0;
	p[at] = (uint8_t)(0x100 - acpi_sum(p, len));
}

static uint8_t	*table_at(uint8_t *img, uint64_t size, uint64_t phys,
		uint32_t *len)
{
	uint64_t	off;

	if (phys < ACPI_FUZZ_BASE || phys - ACPI_FUZZ_BASE > size)
		return (NULL);
	off = phys - ACPI_FUZZ_BASE;
	if (off + ACPI_HDR_LEN > size)
		return (NULL);
	*len = (uint32_t)acpi_rd(img + off, 4, 4);
	if (*len < ACPI_HDR_LEN || *len > size - off)
		return (NULL);
	return (img + off);
}

static void	repair_table(uint8_t *img, uint64_t size, uint64_t phys)
{
	uint8_t		*t;
	uint32_t	len;

	t = table_at(img, size, phys, &len);
	if (t != NULL)
		fix_sum(t, len, 9);
}

static void	repair_root(uint8_t *img, uint64_t size, uint64_t phys,
		uint32_t esize)
{
	uint8_t		*r;
	uint32_t	len;
	uint32_t	rlen;
	uint32_t	i;
	uint64_t	entry;

	r = table_at(img, size, phys, &rlen);
	if (r == NULL)
		return ;
	fix_sum(r, rlen, 9);
	i = 0;
	while (ACPI_HDR_LEN + (i + 1) * esize <= rlen && i < 32)
	{
		entry = acpi_rd(r, ACPI_HDR_LEN + i * esize, esize);
		repair_table(img, size, entry);
		if (table_at(img, size, entry, &len) != NULL
			&& acpi_sig_eq(img + (entry - ACPI_FUZZ_BASE), "FACP")
			&& len >= FADT_DSDT + 4)
			repair_table(img, size, acpi_rd(img + (entry - ACPI_FUZZ_BASE),
					FADT_DSDT, 4));
		i++;
	}
}

static void	repair(uint8_t *img, uint64_t size)
{
	if (size < ACPI_RSDP_V2_LEN)
		return ;
	memcpy(img, "RSD PTR ", 8);
	fix_sum(img, ACPI_RSDP_V1_LEN, 8);
	if (img[RSDP_REVISION] >= 2)
	{
		img[RSDP_LENGTH] = ACPI_RSDP_V2_LEN;
		img[RSDP_LENGTH + 1] = 0;
		img[RSDP_LENGTH + 2] = 0;
		img[RSDP_LENGTH + 3] = 0;
		fix_sum(img, ACPI_RSDP_V2_LEN, 32);
		if (acpi_rd(img, RSDP_XSDT, 8) != 0)
		{
			repair_root(img, size, acpi_rd(img, RSDP_XSDT, 8), 8);
			return ;
		}
	}
	repair_root(img, size, acpi_rd(img, RSDP_RSDT, 4), 4);
}

static void	check_info(const t_acpi_info *i, const t_acpi_extra *x,
		const t_acpi_set *set)
{
	FUZZ_REQUIRE(i->ncpus <= ACPI_MAX_CPUS);
	FUZZ_REQUIRE(i->nioapics <= ACPI_MAX_IOAPICS);
	FUZZ_REQUIRE(i->noverrides <= ACPI_MAX_OVERRIDES);
	FUZZ_REQUIRE(i->nmcfg <= ACPI_MAX_MCFG);
	FUZZ_REQUIRE(x->nnmi <= ACPI_MAX_NMI && x->nnmi_src <= ACPI_MAX_NMI);
	FUZZ_REQUIRE(set->count <= ACPI_TABLES_MAX);
	if (i->poweroff_ok)
		FUZZ_REQUIRE(i->slp_typ_a <= SLP_TYP_MAX
			&& i->slp_typ_b <= SLP_TYP_MAX);
}

static void	run_acpi(const uint8_t *img, uint64_t size)
{
	t_acpi_src		src;
	t_acpi_root		root;
	t_acpi_set		set;
	t_acpi_info		info;
	t_acpi_extra	extra;
	uint16_t		a;
	uint16_t		b;

	g_win.raw = img;
	g_win.size = size;
	src.map = win_map;
	src.unmap = win_unmap;
	src.ctx = NULL;
	memset(&root, 0, sizeof(root));
	memset(&set, 0, sizeof(set));
	memset(&info, 0, sizeof(info));
	memset(&extra, 0, sizeof(extra));
	if (acpi_rsdp_parse(&src, ACPI_FUZZ_BASE, &root) == E_OK)
	{
		acpi_collect(&src, &root, &set);
		acpi_parse_all(&set, &info, &extra);
		check_info(&info, &extra, &set);
	}
	acpi_find_s5(img, size, &a, &b);
	win_release();
}

int	LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
	uint8_t	*copy;

	if (size > ACPI_FUZZ_MAX)
		return (0);
	run_acpi(data, size);
	if (size < ACPI_RSDP_V2_LEN)
		return (0);
	copy = malloc(size);
	if (copy == NULL)
		abort();
	memcpy(copy, data, size);
	repair(copy, size);
	run_acpi(copy, size);
	free(copy);
	return (0);
}
