#include <stdint.h>
#include <stdio.h>
#include "fake_f4.h"
#include "harness.h"
#include "velum/err.h"

static int	prefix_expect(size_t argc, size_t envc, size_t pairs, size_t k)
{
	size_t	aux;

	aux = argc + 3 + envc;
	if (k < 3)
		return (E_INVAL);
	if (k < aux)
		return (E_PROTO);
	if (k >= aux + 2 * pairs + 1)
		return (START_OK);
	return (START_TRUNCATED);
}

static int	prefix_one(const t_f4_stack *s, const size_t *shape, size_t k)
{
	t_startinfo	info;
	int			rc;
	int			want;

	rc = f4_parse(s, k, &info);
	want = prefix_expect(shape[0], shape[1], shape[2], k);
	if (rc == want && (rc < 0
			|| (info.argc == shape[0] && info.envc == shape[1])))
		return (0);
	fprintf(stderr, "prefixe %zu : obtenu %d attendu %d\n", k, rc, want);
	return (1);
}

static void	prefix_run(size_t argc, size_t envc, size_t pairs)
{
	t_f4_stack	s;
	size_t		shape[3];
	size_t		k;
	int			bad;

	shape[0] = argc;
	shape[1] = envc;
	shape[2] = pairs;
	f4_reset(&s);
	f4_head(&s, argc, envc);
	f4_pairs(&s, pairs, 0x1234);
	f4_push(&s, AT_NULL);
	f4_push(&s, 0);
	bad = 0;
	k = 0;
	while (k <= s.n)
		bad += prefix_one(&s, shape, k++);
	h_eq_i64("prefixes errones", bad, 0);
}

static void	prefix_all_stacks(void)
{
	prefix_run(0, 0, 0);
	prefix_run(0, 0, 7);
	prefix_run(3, 0, 7);
	prefix_run(1, 2, 7);
	prefix_run(3, 2, 7);
	prefix_run(256, 0, 7);
	prefix_run(0, 256, 7);
	prefix_run(256, 256, 63);
}

int	main(void)
{
	h_begin("a14/start_parse_prefix");
	h_run("start_parse/valeur limite : tous les prefixes de 8 piles",
		prefix_all_stacks);
	return (h_end());
}
