#include <stdlib.h>
#include <string.h>
#include "../../../lib/crypto/chacha20.h"
#include "a09_test.h"
#include "harness.h"

static void	check_line(const t_vecline *l)
{
	uint8_t		key[CHACHA20_KEY_LEN];
	uint8_t		nonce[CHACHA20_NONCE_LEN];
	uint8_t		out[CHACHA20_BLOCK_LEN];
	t_chacha20	c;

	h_true(l->n == 5, "ligne de vecteur a cinq champs");
	if (l->n != 5)
		return ;
	h_eq_i64("cle de 32 octets", hex_decode(l->f[1], key, sizeof(key)), 32);
	h_eq_i64("nonce de 12 octets", hex_decode(l->f[3], nonce, 12), 12);
	chacha20_init(&c, key, nonce, (uint32_t)strtoul(l->f[2], NULL, 10));
	chacha20_block(&c, out);
	h_eq_hex(l->f[0], out, sizeof(out), l->f[4]);
}

static void	chacha20_vecteurs_rfc8439(void)
{
	int	lines;

	lines = vec_foreach(VEC_DIR "chacha20_block.txt", check_line);
	h_true(lines >= 7, "fichier de vecteurs chacha20 lu en entier");
}

static void	chacha20_compteur_avance_et_boucle(void)
{
	uint8_t		key[CHACHA20_KEY_LEN];
	uint8_t		nonce[CHACHA20_NONCE_LEN];
	uint8_t		seq[CHACHA20_BLOCK_LEN];
	uint8_t		ref[CHACHA20_BLOCK_LEN];
	t_chacha20	c;

	fill_pattern(key, sizeof(key), 8439);
	memset(nonce, 7, sizeof(nonce));
	chacha20_init(&c, key, nonce, 0xffffffffu);
	chacha20_block(&c, seq);
	h_eq_u64("compteur apres un bloc (modulo 2^32)", c.state[12], 0);
	chacha20_block(&c, seq);
	chacha20_init(&c, key, nonce, 0);
	chacha20_block(&c, ref);
	h_true(!memcmp(seq, ref, sizeof(seq)), "bloc 2^32 identique au bloc 0");
	h_eq_u64("compteur apres init a 0 et un bloc", c.state[12], 1);
}

int	main(void)
{
	h_begin("a09/chacha20");
	h_run("chacha20/rfc8439-2.3.2-et-annexe-a1", chacha20_vecteurs_rfc8439);
	h_run("chacha20/compteur-limite-32-bits",
		chacha20_compteur_avance_et_boucle);
	return (h_end());
}
