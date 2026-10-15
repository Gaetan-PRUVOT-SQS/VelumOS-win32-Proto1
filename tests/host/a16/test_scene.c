#include <stdio.h>
#include <stdlib.h>
#include "a16_test.h"

#define SCENE_W 360
#define SCENE_H 200
#define PPM_SCALE 3

#define S_SHADOW 0
#define S_TITLE 1
#define S_UI 2
#define S_UI_WHITE 3
#define S_BOLD 4
#define S_MONO 5
#define S_MONO_WHITE 6

static const uint64_t	g_scene_golden = 0x05916f7e7cca29edull;

static const t_style	g_styles[] = {
{FONT_TITLE, 0xFF0A246A},
{FONT_TITLE, 0xFFFFFFFF},
{FONT_UI, FAKE_INK},
{FONT_UI, 0xFFFFFFFF},
{FONT_UI_BOLD, FAKE_INK},
{FONT_MONO, 0xFFC0C0C0},
{FONT_MONO, 0xFFFFFFFF},
};

static void	put_text(t_surface *s, int style, t_point at, const char *text)
{
	t_textreq	rq;

	req_set(&rq, g_styles[style].font, at, text);
	rq.color = g_styles[style].color;
	font_draw(s, &rq);
}

static void	paint_chrome(t_surface *s)
{
	fill_rect(s, rect_of(0, 0, SCENE_W, SCENE_H), 0xFFECE9D8);
	fill_rect(s, rect_of(0, 0, SCENE_W, 30), 0xFF0058EE);
	put_text(s, S_SHADOW, point_of(9, 8), "Poste de travail");
	put_text(s, S_TITLE, point_of(8, 7), "Poste de travail");
	put_text(s, S_UI, point_of(8, 36),
		"Fichier  Édition  Affichage  Favoris  Outils  ?");
}

static void	paint_content(t_surface *s)
{
	fill_rect(s, rect_of(8, 58, 150, 15), 0xFF316AC5);
	put_text(s, S_UI_WHITE, point_of(12, 59), "Éteindre l'ordinateur");
	put_text(s, S_UI, point_of(12, 76), "Démarrer");
	ellipsis(s, rect_of(12, 93, 140, 15), FONT_UI,
		"Un très long nom de fichier qui ne tient pas dans la boîte");
	put_text(s, S_BOLD, point_of(12, 116), "Paramètres d'affichage");
	button_draw(s, rect_of(12, 138, 75, 23), "Annuler");
	fill_rect(s, rect_of(180, 58, 172, 130), 0xFF000000);
	put_text(s, S_MONO, point_of(184, 62), "C:\\> echo « Salut »");
	put_text(s, S_MONO, point_of(184, 78), "« Salut » … œuvre €");
	put_text(s, S_MONO_WHITE, point_of(184, 94), "C:\\> _");
}

static void	scene_xp_like_window(void)
{
	t_surface	s;
	char		path[256];
	const char	*dir;

	fake_alloc(&s, SCENE_W, SCENE_H);
	paint_chrome(&s);
	paint_content(&s);
	h_eq_i64("SCENE-aucun appel hors surface", g_fake.outside, 0);
	dir = getenv("A16_OUT");
	if (!dir || !*dir)
		dir = "build/a16/render";
	snprintf(path, sizeof(path), "%s/scene.ppm", dir);
	if (fake_write_ppm(&s, path, PPM_SCALE) != 0)
		printf("a16/scene : ecriture de %s impossible (ignore)\n", path);
	printf("a16/scene : hash 0x%016llx\n", (unsigned long long)fake_hash(&s));
	h_eq_u64("SCENE-fenetre d'essai figee par hash", fake_hash(&s),
		g_scene_golden);
	free(s.px);
}

int	main(void)
{
	h_begin("a16/scene");
	h_run("fenetre d'essai de type XP", scene_xp_like_window);
	return (h_end());
}
