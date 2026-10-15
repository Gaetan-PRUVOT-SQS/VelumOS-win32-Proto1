#include <stdint.h>
#include "alloc_int.h"
#include "errno.h"
#include "fake_f3.h"
#include "fake_sys.h"
#include "harness.h"
#include "stdlib.h"
#include "string.h"

static char	g_long[3001];

static void	dup_alloc_failure(void)
{
	memset(g_long, 'x', 3000);
	fake_reset();
	fake_kernel_on();
	g_fsys.valloc_budget = 0;
	errno = 0;
	h_true(strdup("abc") == NULL, "strdup : petit bloc sans memoire");
	h_eq_i64("errno ENOMEM", errno, ENOMEM);
	errno = 0;
	h_true(strndup("abc", 2) == NULL, "strndup : petit bloc sans memoire");
	h_eq_i64("errno ENOMEM (strndup)", errno, ENOMEM);
	errno = 0;
	h_true(strdup(g_long) == NULL, "strdup : gros bloc sans memoire");
	h_eq_i64("errno ENOMEM (gros)", errno, ENOMEM);
	g_fsys.valloc_budget = -1;
}

static void	dup_cases(void)
{
	t_vheapstats	before;
	t_vheapstats	after;
	char			src[8];
	char			*p;

	fake_reset();
	fake_kernel_on();
	v_heap_stats(&before);
	p = strdup("");
	h_true(p != NULL && p[0] == '\0', "chaine vide");
	free(p);
	strlcpy(src, "hello", sizeof(src));
	p = strdup(src);
	h_true(p != NULL && p != src && strcmp(p, "hello") == 0, "copie exacte");
	src[0] = 'J';
	h_true(p[0] == 'h', "copie independante");
	free(p);
	p = strdup(g_long);
	h_true(p != NULL && strlen(p) == 3000, "gros bloc");
	free(p);
	v_heap_stats(&after);
	h_eq_u64("aucune fuite", after.live_blocks, before.live_blocks);
}

int	main(void)
{
	h_begin("a14/strdup");
	h_run("strdup, strndup/injection : panne d'allocation", dup_alloc_failure);
	h_run("strdup/partition : vide, copie, gros, fuite", dup_cases);
	return (h_end());
}
