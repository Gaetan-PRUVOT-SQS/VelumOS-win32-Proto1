#include "display_int.h"
#include "velum/err.h"
#include "velum/random.h"
#include "velum/util.h"
#include "velum/vmm.h"

uint64_t	umap_len(void)
{
	return (align_up(g_display.info.fb_size, PAGE_SIZE));
}

bool	umap_hint_ok(uint64_t hint, uint64_t len)
{
	if (!is_aligned(hint, PAGE_SIZE))
		return (false);
	return (hint >= USER_MIN && hint <= USER_TOP - len);
}

int	umap_place(t_aspace *as, uint64_t hint, uint64_t len, uint64_t *va)
{
	uint64_t	lo;

	*va = 0;
	if (hint && vmm_find_free(as, len, hint, hint + len) == hint)
		*va = hint;
	if (!*va)
	{
		lo = UMAP_RANDOM_LO;
		lo += krandom_below(UMAP_RANDOM_COUNT) * UMAP_RANDOM_ALIGN;
		*va = vmm_find_free(as, len, lo, USER_TOP);
	}
	if (!*va)
		*va = vmm_find_free(as, len, USER_MIN, USER_TOP);
	if (*va < USER_MIN || *va > USER_TOP - len)
		return (E_NOMEM);
	return (E_OK);
}
