#include "harness.h"
#include "fake.h"

static void	valid_methods(void)
{
	h_eq_i64("boucle", fake_v(fake_sample(0)), 0);
	h_eq_i64("boucle debuts", g_c.starts[0], 0x2b);
	h_eq_i64("packed-switch avec bourrage", fake_v(fake_sample(1)), 0);
	h_eq_i64("packed debuts sans charge ni bourrage", g_c.starts[0], 0x19);
	h_eq_i64("packed debuts suite", g_c.starts[1], 0);
	h_eq_i64("sparse-switch et tableau", fake_v(fake_sample(2)), 0);
	h_eq_i64("sparse debuts", g_c.starts[0], 0xc9);
	h_eq_i64("sparse debuts suite", g_c.starts[1] | g_c.starts[2], 0);
	h_eq_i64("appels et paire", fake_v(fake_sample(3)), 0);
	h_eq_i64("appels debuts", g_c.starts[0], 0x65);
	h_eq_i64("appels debuts suite", g_c.starts[1], 0x0a);
	h_eq_i64("appels paire hors bornes", fake_v(2), E_RANGE);
}

static void	payload_switch(void)
{
	t_dswitch	sw;
	t_darray	ar;
	t_span		s;

	fake_sample(1);
	s.p = g_c.b;
	s.len = 2 * g_c.n;
	h_eq_i64("packed lu", dexcode_switch(s, 6, &sw), 0);
	h_eq_i64("packed genre", sw.sparse, 0);
	h_eq_i64("packed cas", sw.count, 2);
	h_eq_i64("packed cible 1", dexcode_rd32(sw.targets + 4), 4);
	h_eq_i64("packed impair refuse", dexcode_switch(s, 5, &sw), E_INVAL);
	h_eq_i64("packed pas un tableau", dexcode_array_data(s, 6, &ar), E_INVAL);
}

static void	payload_sparse_array(void)
{
	t_dswitch	sw;
	t_darray	ar;
	t_span		s;

	fake_sample(2);
	s.p = g_c.b;
	s.len = 2 * g_c.n;
	h_eq_i64("sparse lu", dexcode_switch(s, 8, &sw), 0);
	h_eq_i64("sparse genre", sw.sparse, 1);
	h_eq_i64("sparse premiere cle", sw.first_key, -5);
	h_eq_i64("sparse cle 1", dexcode_rd32(sw.keys + 4), 10);
	h_eq_i64("sparse cible 1", dexcode_rd32(sw.targets + 4), 7);
	h_eq_i64("tableau lu", dexcode_array_data(s, 18, &ar), 0);
	h_eq_i64("tableau largeur", ar.width, 2);
	h_eq_i64("tableau compte", ar.count, 3);
	h_eq_i64("tableau donnee", ar.data[4], 3);
	h_eq_i64("tableau pris pour switch", dexcode_switch(s, 18, &sw), E_INVAL);
	h_eq_i64("hors du code", dexcode_array_data(s, 26, &ar), E_INVAL);
}

static void	truncated_everywhere(void)
{
	t_dcodelimits	l;
	uint32_t		k;
	int				i;
	int				ok;

	i = 0;
	ok = 0;
	l = fake_lim(3);
	while (i < 4)
	{
		fake_sample(i);
		k = 0;
		while (k < g_c.n)
			ok += (fake_exact(g_c.b, k++, &l) == 0);
		i++;
	}
	h_eq_i64("prefixes stricts acceptes", ok, 0);
}

int	main(void)
{
	h_begin("d04 verification structurelle");
	h_run("methodes valides assemblees a la main", valid_methods);
	h_run("lecture d une charge packed", payload_switch);
	h_run("lecture des charges sparse et tableau", payload_sparse_array);
	h_run("troncature a chaque unite", truncated_everywhere);
	return (h_end());
}
