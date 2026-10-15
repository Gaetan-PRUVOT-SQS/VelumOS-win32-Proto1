#include "harness.h"
#include "blk_int.h"

static void	crc_vectors(void)
{
	h_eq_u64("vide", crc32_calc("", 0), 0);
	h_eq_u64("123456789", crc32_calc("123456789", 9), 0xcbf43926u);
	h_eq_u64("a", crc32_calc("a", 1), 0xe8b7be43u);
	h_eq_u64("renard", crc32_calc(
			"The quick brown fox jumps over the lazy dog", 43), 0x414fa339u);
}

static void	crc_chunks(void)
{
	static uint8_t	buf[1000];
	uint32_t		st;
	size_t			cut;
	int				same;

	cut = 0;
	while (cut < sizeof(buf))
	{
		buf[cut] = (uint8_t)(cut * 7 + 3);
		cut++;
	}
	same = 1;
	cut = 0;
	while (cut <= sizeof(buf))
	{
		st = crc32_step(CRC32_INIT, buf, cut);
		st = crc32_step(st, buf + cut, sizeof(buf) - cut);
		same &= (~st == crc32_calc(buf, sizeof(buf)));
		cut += 37;
	}
	h_true(same, "decoupage metamorphique identique au calcul d'un bloc");
}

int	main(void)
{
	h_begin("a11/crc32");
	h_run("vecteurs connus", crc_vectors);
	h_run("decoupage", crc_chunks);
	return (h_end());
}
