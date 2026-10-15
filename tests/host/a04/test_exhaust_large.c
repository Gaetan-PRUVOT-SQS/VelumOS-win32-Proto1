#include <stdint.h>
#include "a04_fake.h"

static void	exhaust_large_arena(void)
{
	void	*b[4];
	int		i;

	a04_fresh_with(14, 8);
	i = 0;
	while (i < 4)
		b[i++] = kmalloc(3000);
	h_true(b[3] != NULL, "E8 quatre blocs d'une page et leur garde");
	h_true(kmalloc(3000) == NULL, "E8 arene des gros blocs pleine");
	h_true(kmalloc(100000) == NULL, "E8 plus grand que l'arene");
	i = 0;
	while (i < 4)
		kfree(b[i++]);
	a04_drain("E8 arene des gros blocs");
}

static void	exhaust_large_fragmentation(void)
{
	void	*b[4];
	void	*q;
	int		i;

	a04_fresh_with(14, 8);
	i = 0;
	while (i < 4)
		b[i++] = kmalloc(3000);
	kfree(b[0]);
	kfree(b[2]);
	h_true(kmalloc(5000) == NULL, "E8 fragmentation : 2 pages et garde");
	kfree(b[1]);
	q = kmalloc(8000);
	h_true(q != NULL, "E8 trous fusionnes : 2 pages tiennent");
	kfree(q);
	kfree(b[3]);
	a04_drain("E8 fragmentation");
}

int	main(void)
{
	h_begin("a04/epuisement-gros");
	h_run("E8 arene des gros blocs", exhaust_large_arena);
	h_run("E8 fragmentation", exhaust_large_fragmentation);
	return (h_end());
}
