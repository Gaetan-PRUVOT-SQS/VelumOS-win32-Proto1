#include <stdint.h>
#include "fake_sys.h"
#include "harness.h"
#include "stdlib.h"
#include "string.h"
#include "velum/vheap.h"

static void	ndup_check(const char *src, size_t max, const char *want)
{
	char	*p;

	p = strndup(src, max);
	h_eq_str("strndup", p, want);
	free(p);
}

static void	ndup_cases(void)
{
	t_vheapstats	before;
	t_vheapstats	after;
	char			*g;

	fake_reset();
	fake_kernel_on();
	v_heap_stats(&before);
	ndup_check("hello", 3, "hel");
	ndup_check("hello", 5, "hello");
	ndup_check("hello", 10, "hello");
	ndup_check("hello", 0, "");
	ndup_check("", 4, "");
	g = fake_guarded(3);
	memcpy(g, "abc", 3);
	ndup_check(g, 3, "abc");
	fake_guarded_free(g, 3);
	v_heap_stats(&after);
	h_eq_u64("aucune fuite", after.live_blocks, before.live_blocks);
}

static int	bz_bad(const uint8_t *b, size_t from, size_t to, uint8_t want)
{
	int	bad;

	bad = 0;
	while (from < to)
		bad += b[from++] != want;
	return (bad);
}

static void	bzero_cases(void)
{
	uint8_t	buf[16];
	uint8_t	*g;
	int		bad;

	memset(buf, 0xff, sizeof(buf));
	explicit_bzero(buf, 0);
	explicit_bzero(NULL, 0);
	h_eq_i64("n = 0 : rien ne change", bz_bad(buf, 0, 16, 0xff), 0);
	explicit_bzero(buf + 4, 8);
	bad = bz_bad(buf, 0, 4, 0xff) + bz_bad(buf, 4, 12, 0);
	bad += bz_bad(buf, 12, 16, 0xff);
	h_eq_i64("seuls les octets 4 a 11 sont a zero", bad, 0);
	g = fake_guarded(16);
	memset(g, 0xaa, 16);
	explicit_bzero(g, 16);
	h_eq_i64("tampon de taille exacte, borne respectee", bz_bad(g, 0, 16, 0),
		0);
	fake_guarded_free(g, 16);
}

int	main(void)
{
	h_begin("a14/strdup_bzero");
	h_run("strndup/limite : max 0, egal, plus grand, sans NUL", ndup_cases);
	h_run("explicit_bzero/limite : n = 0, plage, borne", bzero_cases);
	return (h_end());
}
