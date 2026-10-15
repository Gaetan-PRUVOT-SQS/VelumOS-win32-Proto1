#include <stdio.h>
#include <stdlib.h>
#include "a16_test.h"

#define PAGE_W 520
#define PAGE_LINES 7
#define PPM_SCALE 3

static const char		*g_lines[PAGE_LINES] = {
	"Démarrer",
	"Poste de travail",
	"Éteindre l'ordinateur",
	"Fichier  Édition  Affichage  Favoris  Outils  ?",
	"ÀÂÇÉÈÊËÎÏÔÙÛÜŸ àâçéèêëîïôùûüÿ œŒ € « » … – — ‘ ’ “ ”",
	"ĀăĄćČďĐēĘěĞĢīİıŁńŇŌőřŚŞŠţŤūůŰŹŻž ĲĳĸŉŊŋſ",
	"The quick brown fox jumps over the lazy dog 0123456789 (){}[]<>",
};

static const uint64_t	g_golden[FONT_IDS] = {
	0x303729739d13d41eull, 0xcbe6877fa50be96eull, 0x3929e94c69ff03c6ull,
	0x05adfd93009742c5ull, 0x4f89442afdcdaba2ull, 0xf459f26bc71e06f2ull,
	0x67f5c0407e3d30d4ull, 0x79d5869eede6c3b8ull, 0x015d1f2088e5834dull,
	0x1fb606b946836606ull, 0x4296a2793e7aaa03ull, 0x0e1ea694ba10de3eull,
	0x6cfb70df80d36252ull, 0x77c17c695adb6097ull, 0x01b4172578fe8434ull,
	0xa1f553f05afcd8eaull, 0xa15fe4a39068ccacull, 0xf9f49d60326391b4ull,
	0x657a8e275c913c36ull, 0xcebf144545cec4a6ull, 0xdaf58a35c3841b16ull,
};
static const char		*g_names[FONT_IDS] = {
	"ui", "ui_bold", "title", "mono", "ins_11", "ins_12", "ins_semi_12",
	"ins_13", "ins_semi_13", "ins_bold_13", "ins_14", "ins_semi_14",
	"ins_bold_14", "ins_15", "ins_22", "ins_semi_22", "ins_bold_24",
	"ins_bold_26", "ins_bold_28", "ins_semi_120", "jbm_14",
};

static void	draw_page(t_surface *s, t_fontid id)
{
	t_textreq	rq;
	int32_t		line;

	line = 0;
	while (line < PAGE_LINES)
	{
		req_set(&rq, id, point_of(6, 4 + line * (font_get(id)->height + 2)),
			g_lines[line]);
		font_draw(s, &rq);
		line++;
	}
}

static void	save_ppm(const t_surface *s, const char *name)
{
	char		path[256];
	const char	*dir;

	dir = getenv("A16_OUT");
	if (!dir || !*dir)
		dir = "build/a16/render";
	snprintf(path, sizeof(path), "%s/%s.ppm", dir, name);
	if (fake_write_ppm(s, path, PPM_SCALE) != 0)
		printf("a16/render : ecriture de %s impossible (ignore)\n", path);
}

static void	render_every_font(void)
{
	int			id;
	t_surface	s;
	int32_t		height;

	id = 0;
	while (id < FONT_IDS)
	{
		height = 8 + PAGE_LINES * (font_get((t_fontid)id)->height + 2);
		fake_alloc(&s, PAGE_W, height);
		draw_page(&s, (t_fontid)id);
		h_eq_i64("RENDU-aucun appel hors surface", g_fake.outside, 0);
		save_ppm(&s, g_names[id]);
		printf("a16/render : %s hash 0x%016llx\n", g_names[id],
			(unsigned long long)fake_hash(&s));
		h_eq_u64("RENDU-page francaise figee par hash", fake_hash(&s),
			g_golden[id]);
		free(s.px);
		id++;
	}
}

int	main(void)
{
	h_begin("a16/render");
	h_run("pages d'essai francaises des 21 polices", render_every_font);
	return (h_end());
}
