#include <stdlib.h>
#include "a16_test.h"

#define BLOB_LEN 6

static t_glyph	g_bound_glyphs[] = {
{0x41, 8, 3, 0, 0, 9, 3},
{0xFFFD, 0, 0, 0, 0, 1, 0},
};

static int32_t	calls_with(uint16_t off, uint8_t h, uint32_t nbits)
{
	t_surface	s;
	t_textreq	rq;
	t_font		f;
	uint8_t		*blob;
	int32_t		calls;

	blob = malloc(BLOB_LEN);
	if (!blob)
		abort();
	memset(blob, 0xFF, BLOB_LEN);
	g_bound_glyphs[0].off = off;
	g_bound_glyphs[0].h = h;
	f = (t_font){8, 0, 8, 2, g_bound_glyphs, blob, nbits};
	fake_alloc(&s, RENDER_W, RENDER_H);
	req_set(&rq, FONT_UI, point_of(3, 4), "A");
	rq.font = &f;
	font_draw(&s, &rq);
	calls = g_fake.calls + g_fake.outside;
	free(s.px);
	free(blob);
	return (calls);
}

static void	bounds_offset_limits(void)
{
	h_eq_i64("BORNE-bitmap en fin de table", calls_with(3, 3, BLOB_LEN), 24);
	h_eq_i64("BORNE-bitmap au debut", calls_with(0, 3, BLOB_LEN), 24);
	h_eq_i64("BORNE-table entiere", calls_with(0, 6, BLOB_LEN), 48);
	h_eq_i64("BORNE-depasse d'un octet", calls_with(4, 3, BLOB_LEN), 0);
	h_eq_i64("BORNE-hauteur de trop", calls_with(0, 7, BLOB_LEN), 0);
	h_eq_i64("BORNE-decalage egal a la taille", calls_with(6, 1, BLOB_LEN), 0);
	h_eq_i64("BORNE-decalage apres la table", calls_with(7, 1, BLOB_LEN), 0);
	h_eq_i64("BORNE-decalage maximal", calls_with(0xFFFF, 255, BLOB_LEN), 0);
	h_eq_i64("BORNE-table declaree vide", calls_with(0, 1, 0), 0);
	h_eq_i64("BORNE-table plus courte", calls_with(3, 3, 5), 0);
	h_eq_i64("BORNE-dernier octet seul", calls_with(5, 1, BLOB_LEN), 8);
}

static void	bounds_rows_arguments(void)
{
	const uint8_t	*rows;
	uint8_t			blob[BLOB_LEN];
	t_font			f;

	memset(blob, 0, sizeof(blob));
	g_bound_glyphs[0].off = 3;
	g_bound_glyphs[0].h = 3;
	f = (t_font){8, 0, 8, 2, g_bound_glyphs, blob, BLOB_LEN};
	rows = NULL;
	h_true(font_glyph_rows(&f, &g_bound_glyphs[0], &rows), "BORNE-valide");
	h_true(rows == blob + 3, "BORNE-adresse de la premiere rangee");
	h_true(!font_glyph_rows(NULL, &g_bound_glyphs[0], &rows), "BORNE-police");
	h_true(!font_glyph_rows(&f, NULL, &rows), "BORNE-glyphe nul");
	h_true(!font_glyph_rows(&f, &g_bound_glyphs[0], NULL), "BORNE-sortie");
	h_true(!font_glyph_rows(&f, &g_bound_glyphs[1], &rows), "BORNE-sans encre");
	f.bits = NULL;
	h_true(!font_glyph_rows(&f, &g_bound_glyphs[0], &rows), "BORNE-nulle");
}

static void	bounds_real_fonts_fit(void)
{
	int				id;
	int32_t			k;
	const t_font	*f;
	const t_glyph	*g;

	id = 0;
	while (id < FONT_IDS)
	{
		f = font_get((t_fontid)id++);
		k = 0;
		while (k < f->nglyphs)
		{
			g = &f->glyphs[k++];
			h_true(g->off + g->h * ((g->w + 7u) / 8) <= f->nbits,
				"BORNE-glyphe reel dans sa table");
		}
	}
}

int	main(void)
{
	h_begin("a16/font_bounds");
	h_run("decalages aux limites de la table", bounds_offset_limits);
	h_run("arguments de font_glyph_rows", bounds_rows_arguments);
	h_run("polices reelles dans leur table", bounds_real_fonts_fit);
	return (h_end());
}
