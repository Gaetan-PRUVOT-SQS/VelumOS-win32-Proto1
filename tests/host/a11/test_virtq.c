#include <stdlib.h>
#include "harness.h"
#include "velum/err.h"
#include "fake.h"

static uint8_t	*g_mem;
static t_vq		g_q;

static void	vq_order_chains(void)
{
	static const t_vqbuf	b[3] = {{0x1000, 16, false}, {0x2000, 512, true},
	{0x3000, 1, true}};
	uint16_t				last;
	uint32_t				len;
	int						h[3];

	h_eq_i64("init", vq_init(&g_q, g_mem, 0x100000, 8), 0);
	h_true(g_q.avail->flags == VQ_AVAIL_F_NO_INTERRUPT, "sans interruption");
	h[0] = vq_add(&g_q, b, 3);
	h[1] = vq_add(&g_q, b + 2, 1);
	h[2] = vq_add(&g_q, b, 2);
	h_true(g_q.desc[h[0]].flags == VQ_DESC_F_NEXT, "lu puis suivant");
	h_true(g_q.desc[g_q.desc[h[0]].next].flags == (VQ_DESC_F_NEXT
			| VQ_DESC_F_WRITE), "ecrit puis suivant");
	h_eq_u64("libres", g_q.nfree, 2);
	last = 0;
	fk_vq_serve(&g_q, &last, 1);
	h_true(vq_used_pending(&g_q), "travail rendu");
	h_eq_i64("ordre de l'appareil 1", vq_get(&g_q, &len), h[2]);
	h_eq_u64("longueur ecrite", len, 512);
	h_eq_i64("ordre de l'appareil 2", vq_get(&g_q, &len), h[1]);
	h_eq_i64("ordre de l'appareil 3", vq_get(&g_q, &len), h[0]);
	h_eq_u64("longueur chaine 3", len, 513);
	h_eq_i64("vide", vq_get(&g_q, &len), E_AGAIN);
	h_true(g_q.nfree == 8 && g_q.inflight == 0, "tout rendu");
}

static void	vq_rules(void)
{
	static const t_vqbuf	b[3] = {{0x1000, 16, false}, {0x2000, 0, false},
	{0, 1, true}};
	t_vqbuf					wr[2];

	h_eq_i64("init 4", vq_init(&g_q, g_mem, 0x100000, 4), 0);
	h_eq_i64("n nul", vq_add(&g_q, b, 0), E_INVAL);
	h_eq_i64("n > taille", vq_add(&g_q, b, 5), E_INVAL);
	h_eq_i64("longueur nulle", vq_add(&g_q, b, 2), E_INVAL);
	h_eq_i64("adresse nulle", vq_add(&g_q, b + 2, 1), E_INVAL);
	wr[0] = b[0];
	wr[0].write = true;
	wr[1] = b[0];
	h_eq_i64("lu apres ecrit", vq_add(&g_q, wr, 2), E_INVAL);
	h_true(vq_add(&g_q, b, 1) >= 0 && vq_add(&g_q, b, 1) >= 0
		&& vq_add(&g_q, b, 1) >= 0, "trois chaines");
	wr[1].write = true;
	h_eq_i64("epuisement", vq_add(&g_q, wr, 2), E_AGAIN);
	h_eq_i64("taille 0", vq_init(&g_q, g_mem, 0, 0), E_INVAL);
	h_eq_i64("taille 3", vq_init(&g_q, g_mem, 0, 3), E_INVAL);
	h_eq_i64("taille 256", vq_init(&g_q, g_mem, 0, 256), E_INVAL);
	h_eq_i64("memoire nulle", vq_init(&g_q, NULL, 0, 4), E_INVAL);
	h_true(vq_mem_size(VQ_SIZE_MAX) <= 4096, "file maximale dans une page");
	h_eq_u64("taille 128", vq_mem_size(128), 3342);
}

static void	vq_wrap(void)
{
	static const t_vqbuf	b[2] = {{0x1000, 16, false}, {0x3000, 1, true}};
	uint16_t				last;
	int						i;
	int						ok;

	vq_init(&g_q, g_mem, 0x100000, 4);
	g_q.avail_idx = 65530;
	g_q.last_used = 65530;
	g_q.avail->idx = 65530;
	g_q.used->idx = 65530;
	last = 65530;
	ok = 1;
	i = 0;
	while (i++ < 20)
	{
		ok &= (vq_add(&g_q, b, 2) >= 0);
		fk_vq_serve(&g_q, &last, 0);
		ok &= (vq_get(&g_q, NULL) >= 0);
	}
	h_true(ok, "20 requetes a travers l'enroulement");
	h_eq_u64("indice enroule", g_q.avail_idx, 14);
	h_eq_u64("utilise enroule", g_q.last_used, 14);
	h_true(g_q.nfree == 4 && g_q.inflight == 0, "liste libre intacte");
}

static void	vq_hostile(void)
{
	static const t_vqbuf	b[1] = {{0x3000, 1, true}};
	int						h;

	vq_init(&g_q, g_mem, 0x100000, 4);
	h = vq_add(&g_q, b, 1);
	g_q.used->ring[0].id = (uint32_t)h;
	g_q.used->idx = 5;
	h_eq_i64("indice fou", vq_get(&g_q, NULL), E_IO);
	g_q.used->idx = 1;
	g_q.used->ring[0].id = 4;
	h_eq_i64("id hors file", vq_get(&g_q, NULL), E_IO);
	g_q.used->ring[0].id = 0xffffffffu;
	h_eq_i64("id maximal", vq_get(&g_q, NULL), E_IO);
	g_q.used->ring[0].id = (uint32_t)h + 1;
	h_eq_i64("id pas en vol", vq_get(&g_q, NULL), E_IO);
	g_q.used->ring[0].id = (uint32_t)h;
	h_eq_i64("id correct", vq_get(&g_q, NULL), h);
	g_q.used->idx = 2;
	h_eq_i64("rendu sans rien en vol", vq_get(&g_q, NULL), E_IO);
}

int	main(void)
{
	g_mem = aligned_alloc(4096, 4096);
	if (!g_mem)
		return (1);
	h_begin("a11/virtq");
	h_run("ordre et chaines", vq_order_chains);
	h_run("regles", vq_rules);
	h_run("enroulement 16 bits", vq_wrap);
	h_run("appareil hostile", vq_hostile);
	free(g_mem);
	return (h_end());
}
