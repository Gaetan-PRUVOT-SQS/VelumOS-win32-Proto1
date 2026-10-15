#include <stdint.h>
#include "a04_fake.h"

static void	flip_and_check(uint8_t *where, const char *what)
{
	*where ^= 1;
	if (HEAP_DEBUG)
		a04_expect_found(what);
	else
		a04_expect_clean("E10 release : pas de controle de poison");
	*where ^= 1;
	a04_expect_clean("E10 octet restaure");
}

static void	check_free_slot_poison(void)
{
	uint8_t	*p;

	a04_fresh();
	p = kmalloc(40);
	kfree(p);
	flip_and_check(p + 5, "E10 poison d'un objet libre");
	flip_and_check(p, "E10 premier octet libre");
	flip_and_check(p + 47, "E10 dernier octet libre");
	a04_drain("E10 poison");
}

static void	check_live_red_zone(void)
{
	uint8_t	*p;

	a04_fresh();
	p = kmalloc(100);
	flip_and_check(p + 100, "E10 zone rouge : premier octet");
	flip_and_check(p + 111, "E10 zone rouge : dernier octet de remplissage");
	flip_and_check(p + 112, "E10 zone rouge : taille memorisee");
	flip_and_check(p + 127, "E10 zone rouge : temoin");
	kfree(p);
	a04_drain("E10 zone rouge");
}

static void	check_large_red_zone(void)
{
	uint8_t	*p;

	a04_fresh();
	p = kmalloc(5000);
	flip_and_check(p + 5000, "E10 gros bloc : zone rouge");
	flip_and_check(p + 8127, "E10 gros bloc : fin de page");
	kfree(p);
	a04_drain("E10 zone rouge d'un gros bloc");
}

int	main(void)
{
	h_begin("a04/heap_check-poison");
	h_run("E10 poison des objets libres", check_free_slot_poison);
	h_run("E10 zone rouge d'un petit objet", check_live_red_zone);
	h_run("E10 zone rouge d'un gros bloc", check_large_red_zone);
	return (h_end());
}
