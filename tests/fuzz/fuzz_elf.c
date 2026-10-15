#include <string.h>
#include "fuzz.h"
#include "proc_elf.h"
#include "velum/err.h"
#include "velum/vmm.h"

#define ELF_FUZZ_MAX 0x10000u
#define ELF_FUZZ_RELA_MAX 1024u

static void	check_segments(const t_elfinfo *in, uint64_t size)
{
	const t_elfseg	*s;
	uint32_t		i;

	FUZZ_REQUIRE(in->nseg > 0 && in->nseg <= ELF_SEG_MAX);
	i = 0;
	while (i < in->nseg)
	{
		s = &in->seg[i];
		FUZZ_REQUIRE(s->filesz <= s->memsz);
		FUZZ_REQUIRE(s->offset <= size && s->filesz <= size - s->offset);
		FUZZ_REQUIRE(!((s->flags & ELF_PF_W) && (s->flags & ELF_PF_X)));
		FUZZ_REQUIRE(s->vaddr + s->memsz <= USER_TOP);
		FUZZ_REQUIRE(s->vaddr + s->memsz >= s->vaddr);
		if (i > 0)
			FUZZ_REQUIRE(s->vaddr >= in->seg[i - 1].vaddr
				+ in->seg[i - 1].memsz);
		i++;
	}
}

static void	check_entry_and_span(const t_elfinfo *in)
{
	int	x;

	x = elf_find_seg(in, in->entry, 1);
	FUZZ_REQUIRE(x >= 0);
	FUZZ_REQUIRE(in->seg[x].flags & ELF_PF_X);
	FUZZ_REQUIRE(in->span_hi >= in->span_lo);
	FUZZ_REQUIRE(in->span_hi - in->span_lo <= ELF_SPAN_MAX);
	FUZZ_REQUIRE(elf_mem_bytes(in) >= in->span_hi - in->span_lo
		|| in->nseg > 1);
}

static void	check_relocations(const t_elfimg *im, const t_elfinfo *in)
{
	t_elf64_rela	r;
	uint64_t		i;

	FUZZ_REQUIRE(in->rela_count == 0 || in->rela_off <= im->size);
	FUZZ_REQUIRE(in->rela_count <= im->size / sizeof(r));
	i = 0;
	while (i < in->rela_count && i < ELF_FUZZ_RELA_MAX)
	{
		FUZZ_REQUIRE(elf_get_rela(im, in, i, &r) == 0);
		i++;
	}
	FUZZ_REQUIRE(elf_get_rela(im, in, in->rela_count, &r) < 0);
}

static void	check_base(const t_elfinfo *in)
{
	uint64_t	slots;
	uint64_t	base;

	slots = elf_base_slots(in);
	base = elf_pick_base(in, slots);
	if (in->type == ELF_ET_EXEC)
	{
		FUZZ_REQUIRE(slots == 0 && base == 0);
		return ;
	}
	FUZZ_REQUIRE(slots > 0);
	FUZZ_REQUIRE(base + in->span_hi <= ELF_ASLR_HI);
	base = elf_pick_base(in, slots - 1);
	FUZZ_REQUIRE(base + in->span_hi <= ELF_ASLR_HI);
}

int	LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
	t_elfinfo	in;
	t_elfimg	im;
	int			rc;

	if (size > ELF_FUZZ_MAX)
		return (0);
	rc = elf_validate(data, size, &in);
	if (rc < 0)
		return (0);
	im.data = data;
	im.size = size;
	check_segments(&in, size);
	check_entry_and_span(&in);
	check_relocations(&im, &in);
	check_base(&in);
	return (0);
}
