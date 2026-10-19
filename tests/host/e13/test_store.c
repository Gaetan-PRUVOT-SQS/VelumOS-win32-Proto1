#include <limits.h>
#include <stdio.h>
#include <string.h>
#include "velum/libk.h"
#include "harness.h"
#include "velum/err.h"
#include "velum/notif.h"

static t_notif	make(const char *title)
{
	t_notif	n;

	memset(&n, 0, sizeof(n));
	strlcpy(n.app, "Stockage", sizeof(n.app));
	strlcpy(n.title, title, sizeof(n.title));
	strlcpy(n.text, "Le disque (C:) est plein.", sizeof(n.text));
	return (n);
}

static void	post_order_newest_first(void)
{
	t_notifstore	s;
	t_notif			a;
	t_notif			b;

	notif_store_init(&s);
	a = make("A");
	b = make("B");
	h_eq_i64("id 1", notif_store_post(&s, &a), 1);
	h_eq_i64("id 2", notif_store_post(&s, &b), 2);
	h_eq_i64("nombre", notif_store_count(&s), 2);
	h_eq_str("le plus recent d'abord", notif_store_get(&s, 0)->title, "B");
	h_eq_str("puis l'ancien", notif_store_get(&s, 1)->title, "A");
	h_true(notif_store_get(&s, 2) == NULL, "hors borne : NULL");
}

static void	capacity_drops_oldest(void)
{
	t_notifstore	s;
	t_notif			n;
	char			title[16];
	unsigned			i;

	notif_store_init(&s);
	i = 1;
	while (i <= NOTIF_MAX + 1)
	{
		snprintf(title, sizeof(title), "n%u", i);
		n = make(title);
		h_true(notif_store_post(&s, &n) > 0, "depot accepte");
		i++;
	}
	h_eq_i64("plafond", notif_store_count(&s), NOTIF_MAX);
	h_eq_i64("une perte", s.dropped, 1);
	h_eq_str("recent", notif_store_get(&s, 0)->title, "n17");
	h_eq_str("plus ancien conserve", notif_store_get(&s, NOTIF_MAX - 1)->title,
		"n2");
}

static void	dismiss_by_id(void)
{
	t_notifstore	s;
	t_notif			n;

	notif_store_init(&s);
	n = make("A");
	notif_store_post(&s, &n);
	n = make("B");
	notif_store_post(&s, &n);
	n = make("C");
	notif_store_post(&s, &n);
	h_eq_i64("retrait du milieu", notif_store_dismiss(&s, 2), 0);
	h_eq_i64("reste 2", notif_store_count(&s), 2);
	h_eq_str("ordre conserve", notif_store_get(&s, 0)->title, "C");
	h_eq_str("ordre conserve 2", notif_store_get(&s, 1)->title, "A");
	h_eq_i64("id inconnu", notif_store_dismiss(&s, 2), E_NOENT);
	h_eq_i64("id nul", notif_store_dismiss(&s, 0), E_INVAL);
}

static void	clear_keeps_ids_unique(void)
{
	t_notifstore	s;
	t_notif			n;

	notif_store_init(&s);
	n = make("A");
	notif_store_post(&s, &n);
	notif_store_clear(&s);
	h_eq_i64("vide", notif_store_count(&s), 0);
	h_eq_i64("id jamais reutilise", notif_store_post(&s, &n), 2);
}

static void	id_au_dela_de_int_max(void)
{
	t_notifstore	s;
	t_notif			n;
	int				id;
	const t_notif	*slot;

	notif_store_init(&s);
	s.next_id = (uint32_t)INT_MAX + 1u;
	n = make("Haut");
	id = notif_store_post(&s, &n);
	h_true(id > 0, "id positif");
	slot = notif_store_get(&s, 0);
	h_true(slot != NULL && slot->id == (uint32_t)id, "meme id dans la case");
	h_eq_i64("revenu a 1", id, 1);
	h_eq_i64("compteur suivant", s.next_id, 2);
	s.next_id = 0;
	id = notif_store_post(&s, &n);
	slot = notif_store_get(&s, 0);
	h_true(id > 0 && slot != NULL && slot->id == (uint32_t)id, "zero saute");
	h_eq_i64("apres zero", id, 2);
	s.next_id = (uint32_t)INT_MAX + 1u;
	id = notif_store_post(&s, &n);
	slot = notif_store_get(&s, 0);
	h_true(id > 0 && slot != NULL && slot->id == (uint32_t)id,
		"vivant saute, id positif");
	h_eq_i64("1 et 2 sautes", id, 3);
	notif_store_init(&s);
	s.next_id = (uint32_t)INT_MAX;
	n = make("Borne");
	id = notif_store_post(&s, &n);
	h_eq_i64("INT_MAX tient", id, INT_MAX);
	h_eq_i64("compteur repasse a 1", s.next_id, 1);
	slot = notif_store_get(&s, 0);
	h_true(slot != NULL && slot->id == (uint32_t)INT_MAX, "case a INT_MAX");
	id = notif_store_post(&s, &n);
	h_eq_i64("apres INT_MAX, retour a 1", id, 1);
	slot = notif_store_get(&s, 0);
	h_true(slot != NULL && slot->id == (uint32_t)id, "case du retour");
}

static void	invalid_refused(void)
{
	t_notifstore	s;
	t_notif			n;

	notif_store_init(&s);
	n = make("");
	h_eq_i64("titre vide", notif_store_post(&s, &n), E_INVAL);
	n = make("ok");
	n.text[0] = '\n';
	h_eq_i64("caractere de controle", notif_store_post(&s, &n), E_INVAL);
	h_eq_i64("magasin NULL", notif_store_post(NULL, &n), E_INVAL);
	h_eq_i64("rien depose", notif_store_count(&s), 0);
}

int	main(void)
{
	h_begin("e13/store");
	h_run("depot : le plus recent d'abord", post_order_newest_first);
	h_run("plafond 16 : le plus ancien tombe", capacity_drops_oldest);
	h_run("retrait par identifiant", dismiss_by_id);
	h_run("tout effacer : identifiants non reutilises", clear_keeps_ids_unique);
	h_run("au-dela de INT_MAX : id positif", id_au_dela_de_int_max);
	h_run("entrees invalides refusees", invalid_refused);
	return (h_end());
}
