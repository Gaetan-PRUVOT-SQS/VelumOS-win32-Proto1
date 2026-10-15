#include "harness.h"
#include "th.h"

static int	encode(uint8_t *out, uint16_t code, int release)
{
	int	n;

	n = 0;
	if ((code & 0xff00) == 0xe000)
		out[n++] = 0xe0;
	if (release)
		out[n++] = 0xf0;
	out[n++] = (uint8_t)(code & 0xff);
	return (n);
}

static int	gen_stream(uint64_t *rng, t_trip *t)
{
	int	n;
	int	i;

	n = 0;
	i = 0;
	while (i < 1000)
	{
		t->want[i] = (uint16_t)(1 + th_rand(rng) % 0x7f);
		if (t->want[i] == 0x12 || t->want[i] == 0x59)
			t->want[i] = 0x1c;
		if (th_rand(rng) & 1)
			t->want[i] |= 0xe000;
		n += encode(&t->buf[n], t->want[i], (int)(th_rand(rng) & 1));
		i++;
	}
	return (n);
}

static void	round_trip(void)
{
	t_trip		t;
	t_scan2		s;
	uint64_t	rng;
	int			n;
	int			i;

	rng = th_seed("a10/scan2_trip");
	n = gen_stream(&rng, &t);
	scan2_reset(&s);
	h_eq_i64("evenements decodes", th_scan(&s, t.buf, n, t.got), 1000);
	i = 0;
	while (i < 1000 && t.got[i].code == t.want[i])
		i++;
	h_eq_i64("aller retour de 1000 evenements", i, 1000);
}

int	main(void)
{
	h_begin("a10/scan2_trip");
	h_run("aller retour metamorphique", round_trip);
	return (h_end());
}
