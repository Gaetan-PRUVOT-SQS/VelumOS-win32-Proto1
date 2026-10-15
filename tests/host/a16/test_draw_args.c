#include <stdlib.h>
#include "a16_test.h"

static uint64_t	hash_of(const char *text, int32_t len)
{
	t_textreq	rq;

	req_set(&rq, FONT_UI, point_of(3, 4), text);
	rq.len = len;
	return (render_hash(&rq));
}

static void	args_len_semantics(void)
{
	hash_of("AB", 0);
	h_eq_i64("LEN-zero : aucun appel de gfx_put", g_fake.calls, 0);
	h_eq_u64("LEN-un octet", hash_of("AB", 1), hash_of("A", -1));
	h_eq_u64("LEN-negatif = jusqu'au NUL", hash_of("AB", -1), hash_of("AB", 2));
	h_eq_u64("LEN-INT32_MIN", hash_of("AB", INT32_MIN), hash_of("AB", 2));
	h_eq_u64("LEN-plus long que la chaine", hash_of("AB", 10),
		hash_of("AB", 2));
	h_eq_u64("LEN-INT32_MAX", hash_of("AB", INT32_MAX), hash_of("AB", 2));
	h_eq_u64("LEN-NUL integre arrete le dessin", hash_of("A\0B", 3),
		hash_of("A", -1));
	h_eq_u64("LEN-coupe au milieu d'un caractere", hash_of("\xC3\xA9", 1),
		hash_of("\xEF\xBF\xBD", -1));
}

static void	args_null_arguments(void)
{
	t_surface	s;
	t_textreq	rq;

	fake_alloc(&s, RENDER_W, RENDER_H);
	req_set(&rq, FONT_UI, point_of(3, 4), "AB");
	font_draw(NULL, &rq);
	font_draw(&s, NULL);
	rq.font = font_get(FONT_IDS);
	font_draw(&s, &rq);
	rq.font = font_get(FONT_UI);
	rq.text = NULL;
	font_draw(&s, &rq);
	h_eq_i64("ARGS-nul, police inconnue, texte nul : aucun appel",
		g_fake.calls, 0);
	free(s.px);
}

static void	args_empty_targets(void)
{
	t_surface	s;
	t_textreq	rq;
	uint32_t	*keep;

	fake_alloc(&s, RENDER_W, RENDER_H);
	req_set(&rq, FONT_UI, point_of(3, 4), "AB");
	s.clip.w = 0;
	font_draw(&s, &rq);
	s.clip.w = -5;
	font_draw(&s, &rq);
	s.clip.w = RENDER_W;
	s.h = 0;
	font_draw(&s, &rq);
	s.h = RENDER_H;
	s.w = -RENDER_W;
	font_draw(&s, &rq);
	s.w = RENDER_W;
	keep = s.px;
	s.px = NULL;
	font_draw(&s, &rq);
	h_eq_i64("CIBLE-clip vide, dimension <= 0, px nul : aucun appel",
		g_fake.calls, 0);
	free(keep);
}

int	main(void)
{
	h_begin("a16/draw_args");
	h_run("semantique de len", args_len_semantics);
	h_run("arguments nuls", args_null_arguments);
	h_run("cibles vides ou invalides", args_empty_targets);
	return (h_end());
}
