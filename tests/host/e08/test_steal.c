#include "harness.h"
#include "smp_fixture.h"

static void	vide_rien_a_voler(void)
{
	t_runq	rq;

	runq_init(&rq);
	h_true(runq_steal(&rq) == NULL, "file vide : rien");
}

static void	queue_du_niveau_le_plus_haut(void)
{
	t_runq	rq;
	t_tcase	*a;
	t_tcase	*b;
	t_tcase	*c;
	t_tcase	*bas;

	runq_init(&rq);
	a = tcase_new(8, 0);
	b = tcase_new(8, 0);
	c = tcase_new(8, 0);
	bas = tcase_new(2, 0);
	runq_push(&rq, &bas->kt, false);
	runq_push(&rq, &a->kt, false);
	runq_push(&rq, &b->kt, false);
	runq_push(&rq, &c->kt, false);
	h_true(runq_steal(&rq) == &c->kt, "vole la queue du niveau 8");
	h_true(runq_steal(&rq) == &b->kt, "puis le précédent");
	h_true(runq_steal(&rq) == &a->kt, "puis la tête du niveau");
	h_true(runq_steal(&rq) == &bas->kt, "puis le niveau inférieur");
	h_eq_u64("file vidée", rq.count, 0);
	h_eq_u64("bitmap vide", rq.bitmap, 0);
	free(a);
	free(b);
	free(c);
	free(bas);
}

static void	epingles_ignores(void)
{
	t_runq	rq;
	t_tcase	*epingle;
	t_tcase	*statique;
	t_tcase	*inactif;
	t_tcase	*libre;

	runq_init(&rq);
	libre = tcase_new(5, 0);
	epingle = tcase_new(20, KT_PINNED);
	statique = tcase_new(20, KT_STATIC);
	inactif = tcase_new(20, KT_IDLE);
	runq_push(&rq, &libre->kt, false);
	runq_push(&rq, &epingle->kt, false);
	runq_push(&rq, &statique->kt, false);
	runq_push(&rq, &inactif->kt, false);
	h_true(runq_steal(&rq) == &libre->kt, "seul le fil libre est volé");
	h_true(runq_steal(&rq) == NULL, "restent des fils non volables : NULL");
	h_eq_u64("les trois autres restent en file", rq.count, 3);
	free(libre);
	free(epingle);
	free(statique);
	free(inactif);
}

int	main(void)
{
	h_begin("e08/steal");
	h_run("file vide", vide_rien_a_voler);
	h_run("ordre du vol", queue_du_niveau_le_plus_haut);
	h_run("fils épinglés", epingles_ignores);
	return (h_end());
}
