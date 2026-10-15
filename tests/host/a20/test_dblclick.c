#include <stdint.h>
#include "harness.h"
#include "a20_test.h"
#include "desktop.h"

static void	dblclick_sequences_de_base(void)
{
	t_click	last;
	t_click	c;

	dc_reset(&last);
	c = (t_click){1, 100, 100, 1000 * NS_SEC};
	h_eq_i64("premier clic", dc_feed(&last, &c), 0);
	c.at_ns += 200000000ull;
	h_eq_i64("deuxieme clic rapide", dc_feed(&last, &c), 1);
	c.at_ns += 100000000ull;
	h_eq_i64("troisieme: nouveau premier clic", dc_feed(&last, &c), 0);
	c.at_ns += 100000000ull;
	h_eq_i64("quatrieme: double", dc_feed(&last, &c), 1);
	h_eq_i64("etat remis a zero", last.index, DESK_NONE);
}

static void	dblclick_limites_de_temps(void)
{
	t_click	last;
	t_click	c;

	dc_reset(&last);
	c = (t_click){0, 5, 5, 10 * NS_SEC};
	dc_feed(&last, &c);
	c.at_ns += DBL_MAX_NS;
	h_eq_i64("exactement 500 ms", dc_feed(&last, &c), 1);
	dc_feed(&last, &c);
	c.at_ns += DBL_MAX_NS + 1;
	h_eq_i64("500 ms + 1 ns", dc_feed(&last, &c), 0);
	c.at_ns += 1;
	h_eq_i64("1 ns apres", dc_feed(&last, &c), 1);
	dc_feed(&last, &c);
	h_eq_i64("meme instant", dc_feed(&last, &c), 0);
	c.at_ns -= 1;
	h_eq_i64("horloge en arriere", dc_feed(&last, &c), 0);
}

static void	dblclick_distance_et_cible(void)
{
	t_click	last;
	t_click	c;

	dc_reset(&last);
	c = (t_click){2, 50, 50, 10 * NS_SEC};
	dc_feed(&last, &c);
	c.at_ns += NS_SEC / 10;
	c.x += DBL_SLOP;
	c.y -= DBL_SLOP;
	h_eq_i64("4 px en diagonale", dc_feed(&last, &c), 1);
	dc_feed(&last, &c);
	c.at_ns += NS_SEC / 10;
	c.x += DBL_SLOP + 1;
	h_eq_i64("5 px", dc_feed(&last, &c), 0);
	c.at_ns += NS_SEC / 10;
	c.index = 1;
	h_eq_i64("autre icone", dc_feed(&last, &c), 0);
	c.at_ns += NS_SEC / 10;
	c.index = DESK_NONE;
	h_eq_i64("clic dans le vide", dc_feed(&last, &c), 0);
	c.at_ns += NS_SEC / 10;
	h_eq_i64("deux clics dans le vide", dc_feed(&last, &c), 0);
}

int	main(void)
{
	h_begin("a20/dblclick");
	h_run("double-clic: sequences de base", dblclick_sequences_de_base);
	h_run("double-clic: limites de temps", dblclick_limites_de_temps);
	h_run("double-clic: distance et cible", dblclick_distance_et_cible);
	return (h_end());
}
