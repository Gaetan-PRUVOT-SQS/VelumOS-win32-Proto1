#include <stdint.h>
#include "harness.h"
#include "a20_test.h"
#include "tbuf.h"

static void	tbuf_nombres(void)
{
	char	out[32];
	t_tbuf	b;

	tb_init(&b, out, sizeof(out));
	tb_num(&b, 0, 1);
	tb_putc(&b, '|');
	tb_num(&b, 7, 3);
	tb_putc(&b, '|');
	tb_num(&b, 1234, 2);
	tb_putc(&b, '|');
	tb_num(&b, 5, 99);
	h_eq_str("0, 007, 1234, largeur plafonnee", out,
		"0|007|1234|00000000000000000005");
	tb_init(&b, out, sizeof(out));
	tb_num(&b, UINT64_MAX, 1);
	h_eq_str("uint64 max", out, "18446744073709551615");
	h_eq_i64("pas de depassement", tb_result(&b), 20);
}

static void	tbuf_troncature(void)
{
	char	out[4];
	t_tbuf	b;

	tb_init(&b, out, sizeof(out));
	tb_str(&b, "abcdef");
	h_eq_str("tronque", out, "abc");
	h_eq_i64("signale E_RANGE", tb_result(&b), -34);
	tb_init(&b, out, 1);
	tb_str(&b, "x");
	h_eq_str("taille 1", out, "");
	h_eq_i64("taille 1 signale", tb_result(&b), -34);
	tb_init(&b, out, sizeof(out));
	tb_str(&b, "abc");
	h_eq_i64("tient pile", tb_result(&b), 3);
}

static void	tbuf_taille_zero(void)
{
	char	out[2];
	t_tbuf	b;

	out[0] = 'x';
	tb_init(&b, out, 0);
	tb_putc(&b, 'a');
	tb_num(&b, 42, 2);
	tb_str(&b, "zz");
	h_eq_i64("rien ecrit", out[0], 'x');
	h_eq_i64("signale", tb_result(&b), -34);
}

int	main(void)
{
	h_begin("a20/tbuf");
	h_run("tbuf: nombres et largeur", tbuf_nombres);
	h_run("tbuf: troncature", tbuf_troncature);
	h_run("tbuf: taille zero", tbuf_taille_zero);
	return (h_end());
}
