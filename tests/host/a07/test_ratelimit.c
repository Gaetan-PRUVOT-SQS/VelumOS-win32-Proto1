#include "a07_fake.h"
#include "harness.h"

static void	rl_window(void)
{
	t_ratelimit	rl;
	int			i;
	int			ok;

	rl.start_ns = 0;
	rl.count = 0;
	rl.dropped = 0;
	ok = 0;
	i = 0;
	while (i < 100)
		ok += rl_allow(&rl, 5000000000ull + (uint64_t)i++, 100);
	h_eq_i64("limite 100 lignes passent", ok, 100);
	h_eq_i64("ligne 101 refusee", rl_allow(&rl, 5000000100ull, 100), 0);
	h_eq_u64("perte comptee", rl.dropped, 1);
	h_eq_i64("limite fenetre - 1 ns refusee",
		rl_allow(&rl, 5000000000ull + RL_WINDOW_NS - 1, 100), 0);
	h_eq_i64("limite fenetre suivante acceptee",
		rl_allow(&rl, 5000000000ull + RL_WINDOW_NS, 100), 1);
	h_eq_u64("pertes remises a zero", rl.dropped, 0);
}

static void	rl_clock(void)
{
	t_ratelimit	rl;

	rl.start_ns = 9000000000ull;
	rl.count = 100;
	rl.dropped = 7;
	h_eq_i64("horloge qui recule : nouvelle fenetre",
		rl_allow(&rl, 1000, 100), 1);
	h_eq_u64("debut repris", rl.start_ns, 1000);
	rl.count = 0;
	h_eq_i64("plafond nul refuse tout", rl_allow(&rl, 1001, 0), 0);
	rl.dropped = UINT32_MAX;
	rl_allow(&rl, 1002, 0);
	h_eq_u64("compteur de pertes sature", rl.dropped, UINT32_MAX);
}

static void	sanitize_all(void)
{
	char	buf[256];
	int		i;
	int		bad;

	i = 0;
	while (i < 256)
	{
		buf[i] = (char)i;
		i++;
	}
	log_sanitize(buf, 256);
	bad = 0;
	i = 0;
	while (i < 256)
	{
		if ((i < 0x20 || i == 0x7f) != (buf[i] == '?' && i != '?'))
			bad++;
		i++;
	}
	h_eq_i64("256 octets : controles remplaces, reste intact", bad, 0);
}

int	main(void)
{
	h_begin("a07/journal");
	h_run("fenetre du limiteur", rl_window);
	h_run("horloge et saturation", rl_clock);
	h_run("nettoyage des controles", sanitize_all);
	return (h_end());
}
