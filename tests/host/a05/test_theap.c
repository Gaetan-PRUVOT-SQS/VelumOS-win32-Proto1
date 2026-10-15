#include <stdint.h>
#include "harness.h"
#include "fake.h"
#include "time_int.h"
#include "velum/err.h"

#define CAP 10000

static t_tslot	g_slots[CAP];
static uint32_t	g_idx[2 * CAP];
static t_theap	g_heap;
static uint8_t	g_cancel[CAP + 1];
static uint8_t	g_order[5] = {1, 3, 4, 2, 0};

static void	heap_order(void)
{
	t_tslot		t;
	uint32_t	k;
	uint32_t	ok;

	theap_init(&g_heap, g_slots, g_idx, 8);
	h_eq_u64("tas vide", theap_peek(&g_heap), UINT64_MAX);
	theap_insert(&g_heap, 50, ffn_a, (void *)0);
	theap_insert(&g_heap, 10, ffn_a, (void *)1);
	theap_insert(&g_heap, 30, ffn_a, (void *)2);
	theap_insert(&g_heap, 10, ffn_a, (void *)3);
	theap_insert(&g_heap, 20, ffn_a, (void *)4);
	h_eq_u64("plus proche", theap_peek(&g_heap), 10);
	h_true(!theap_pop(&g_heap, 9, &t), "rien d'echu avant 10");
	ok = 1;
	k = 0;
	while (theap_pop(&g_heap, 100, &t))
	{
		ok &= (k < 5 && (uintptr_t)t.ctx == g_order[k]);
		k++;
	}
	h_true(ok && k == 5, "ordre et fifo a echeance egale");
}

static void	heap_cancel(void)
{
	t_tslot	t;
	int64_t	a;
	int64_t	b;
	int64_t	c;

	theap_init(&g_heap, g_slots, g_idx, 3);
	a = theap_insert(&g_heap, 5, ffn_a, NULL);
	b = theap_insert(&g_heap, 6, ffn_b, NULL);
	c = theap_insert(&g_heap, 7, ffn_c, NULL);
	h_true(a > 0 && b > 0 && c > 0 && a != b && b != c, "ids positifs");
	h_eq_i64("capacite pleine", theap_insert(&g_heap, 8, ffn_a, NULL), E_NOMEM);
	h_true(theap_cancel(&g_heap, b), "annulation armee");
	h_true(!theap_cancel(&g_heap, b), "annulation double refusee");
	h_true(theap_pop(&g_heap, 100, &t) && t.fn == ffn_a, "premier echu");
	h_true(!theap_cancel(&g_heap, a), "annulation apres declenchement");
	h_true(!theap_cancel(&g_heap, 0) && !theap_cancel(&g_heap, -1), "ids nuls");
	h_true(!theap_cancel(&g_heap, 4), "case hors capacite");
	h_true(!theap_cancel(&g_heap, c + (1ll << 32)), "generation fausse");
	b = theap_insert(&g_heap, 9, ffn_b, NULL);
	h_true(b > 0 && b != a, "case reprise avec nouvelle generation");
	h_true(!theap_cancel(&g_heap, a), "ancien id de la case refuse");
	h_true(theap_pop(&g_heap, 100, &t) && t.fn == ffn_c, "reste trie");
}

static void	heap_check_pops(uint32_t *popped, uint32_t *bad)
{
	t_tslot		t;
	uint64_t	last;
	uint64_t	last_seq;

	last = 0;
	last_seq = 0;
	while (theap_pop(&g_heap, UINT64_MAX, &t))
	{
		*bad += (t.deadline < last || (t.deadline == last
					&& t.seq < last_seq));
		*bad += g_cancel[(uintptr_t)t.ctx];
		last = t.deadline;
		last_seq = t.seq;
		(*popped)++;
	}
}

static void	heap_random(void)
{
	uint64_t	seed;
	uint32_t	k;
	uint32_t	popped;
	uint32_t	bad;
	int64_t		id;

	seed = 0xa05a05a05ull;
	theap_init(&g_heap, g_slots, g_idx, CAP);
	k = 0;
	bad = 0;
	while (k < CAP)
	{
		seed = seed * 6364136223846793005ull + 1442695040888963407ull;
		id = theap_insert(&g_heap, (seed >> 33) % 5000, ffn_a,
				(void *)(uintptr_t)(k + 1));
		g_cancel[k + 1] = (k % 3 == 0);
		if (g_cancel[k + 1])
			bad += !theap_cancel(&g_heap, id);
		k++;
	}
	popped = 0;
	heap_check_pops(&popped, &bad);
	h_eq_u64("10000 minuteries, ordre et annulations", bad, 0);
	h_eq_u64("declenchees = armees - annulees", popped, CAP - 3334);
	h_eq_u64("toutes les cases rendues", g_heap.nfree, CAP);
}

int	main(void)
{
	h_begin("a05/theap");
	h_run("ordre et fifo", heap_order);
	h_run("annulation et generations", heap_cancel);
	h_run("10000 minuteries aleatoires", heap_random);
	return (h_end());
}
