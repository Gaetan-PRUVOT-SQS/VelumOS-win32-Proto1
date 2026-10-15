#include "harness.h"
#include "fake.h"
#include "ctl_int.h"

static void	two_clicks(t_fakeui *u, int64_t t0, int64_t gap, int dx)
{
	fake_clock_set(t0);
	fake_click(&u->r, 20, 10);
	fake_clock_set(t0 + gap);
	fake_click(&u->r, 20 + dx, 10);
}

static void	time_partitions(void)
{
	t_fakeui	u;
	int			n;

	fake_begin(&u, 200, 100);
	fake_list(&u, 5);
	ctl_set_clock(&u.r, fake_clock_now);
	two_clicks(&u, 1000000000, CTL_DBLCLK_NS, 0);
	n = fake_cmd_find(1, CN_DBLCLK);
	h_eq_i64("exactement 500 ms : double", n, 1);
	two_clicks(&u, 5000000000, CTL_DBLCLK_NS + 1, 0);
	h_eq_i64("500 ms + 1 ns : pas double", fake_cmd_find(1, CN_DBLCLK), n);
	two_clicks(&u, 9000000000, 0, 0);
	h_eq_i64("meme instant : double", fake_cmd_find(1, CN_DBLCLK), n + 1);
	two_clicks(&u, 20000000000, -1000000, 0);
	h_eq_i64("horloge qui recule : pas double", fake_cmd_find(1, CN_DBLCLK),
		n + 1);
	fake_done(&u);
}

static void	distance_partitions(void)
{
	t_fakeui	u;

	fake_begin(&u, 200, 100);
	fake_list(&u, 5);
	ctl_set_clock(&u.r, fake_clock_now);
	two_clicks(&u, 1000000000, 100000000, CTL_DBLCLK_DIST);
	h_eq_i64("4 pixels : double", fake_cmd_find(1, CN_DBLCLK), 1);
	two_clicks(&u, 5000000000, 100000000, CTL_DBLCLK_DIST + 1);
	h_eq_i64("5 pixels : pas double", fake_cmd_find(1, CN_DBLCLK), 1);
	two_clicks(&u, 9000000000, 100000000, -CTL_DBLCLK_DIST);
	h_eq_i64("-4 pixels : double", fake_cmd_find(1, CN_DBLCLK), 2);
	fake_done(&u);
}

static void	sequence_and_no_clock(void)
{
	t_fakeui	u;

	fake_begin(&u, 200, 100);
	fake_list(&u, 5);
	two_clicks(&u, 1000000000, 100000000, 0);
	h_eq_i64("sans horloge : jamais double", fake_cmd_find(1, CN_DBLCLK), 0);
	ctl_set_clock(&u.r, fake_clock_now);
	two_clicks(&u, 5000000000, 100000000, 0);
	fake_clock_set(5200000000);
	fake_click(&u.r, 20, 10);
	h_eq_i64("troisieme clic : pas double", fake_cmd_find(1, CN_DBLCLK), 1);
	fake_clock_set(5300000000);
	fake_click(&u.r, 20, 10);
	h_eq_i64("quatrieme : double", fake_cmd_find(1, CN_DBLCLK), 2);
	ctl_set_clock(&u.r, NULL);
	two_clicks(&u, 9000000000, 1, 0);
	h_eq_i64("horloge retiree", fake_cmd_find(1, CN_DBLCLK), 2);
	fake_done(&u);
}

int	main(void)
{
	h_begin("a19/dbl");
	h_run("double clic : delai", time_partitions);
	h_run("double clic : distance", distance_partitions);
	h_run("double clic : suite, sans horloge", sequence_and_no_clock);
	return (h_end());
}
