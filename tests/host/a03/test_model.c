#include <stdio.h>
#include <string.h>
#include "velum/err.h"
#include "harness.h"
#include "fake.h"
#include "model.h"

static void	model_step(t_model *m)
{
	uint32_t	op;
	uint32_t	a;
	uint32_t	n;
	int			want;

	op = model_rand(m) % 3;
	a = model_rand(m) % MODEL_PAGES;
	n = 1 + model_rand(m) % 4;
	if (a + n > MODEL_PAGES)
		n = MODEL_PAGES - a;
	m->next_prot = VM_R | (model_rand(m) & 1) * VM_W;
	want = model_expect(m, op, a, n);
	if (model_call(op, a, n, m->next_prot) != want)
		m->mismatches++;
	if (want == 0)
		model_update(m, op, a, n);
	model_compare(m);
}

static void	modele_aleatoire(void)
{
	t_model	m;
	int		k;

	fake_reset();
	fake_user();
	memset(&m, 0, sizeof(m));
	m.rng = MODEL_SEED;
	printf("graine du modele : %#x, %d operations\n", MODEL_SEED, MODEL_OPS);
	k = 0;
	while (k < MODEL_OPS)
	{
		model_step(&m);
		k++;
	}
	h_eq_i64("ecarts au modele", m.mismatches, 0);
	vmm_aspace_destroy(g_vmm.current);
	h_eq_u64("frames rendues", g_fake.live[PMM_USER], 0);
	h_eq_u64("tables rendues", g_fake.live[PMM_PAGETABLE], KH_BASELINE);
	fake_clean("faux pmm propre");
}

static void	metamorphique_ordre(void)
{
	t_aspace	*up;
	t_aspace	*down;

	fake_reset();
	up = vmm_aspace_create();
	down = vmm_aspace_create();
	model_fill(up, 0);
	model_fill(down, 1);
	h_eq_u64("meme cout", vmm_pages_used(up), vmm_pages_used(down));
	h_eq_i64("memes drapeaux", model_diff(up, down), 0);
	h_eq_u64("meme trou", vmm_find_free(up, 4096, UVA, USER_TOP),
		vmm_find_free(down, 4096, UVA, USER_TOP));
	vmm_aspace_destroy(up);
	vmm_aspace_destroy(down);
	h_eq_u64("tout rendu", g_fake.live[PMM_USER] + g_fake.live[PMM_KERNEL], 0);
	fake_clean("faux pmm propre");
}

int	main(void)
{
	h_begin("a03/model");
	h_run("modele_aleatoire", modele_aleatoire);
	h_run("metamorphique_ordre", metamorphique_ordre);
	return (h_end());
}
