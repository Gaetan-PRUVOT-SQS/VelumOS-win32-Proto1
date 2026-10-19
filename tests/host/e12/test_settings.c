#include <string.h>
#include "../harness.h"
#include "velum/err.h"
#include "velum/settings.h"

static void	defaults(void)
{
	t_settings	s;

	set_default(&s);
	h_eq_i64("theme par defaut clair", s.theme, SET_THEME_LIGHT);
	h_eq_i64("transparence normale", s.reduced, 0);
	h_eq_i64("clavier azerty", s.layout, SET_LAYOUT_AZERTY);
	h_eq_i64("resolution 1280x720", set_mode_get(s.mode).w, 1280);
}

static void	parse_valid(void)
{
	static const char	txt[] = "theme=sombre\ntransparence=reduite\n"
		"clavier=qwerty\nresolution=1920x1080\n";
	t_settings			s;

	h_eq_i64("4 cles reconnues", set_parse(txt, sizeof(txt) - 1, &s), 4);
	h_eq_i64("theme sombre", s.theme, SET_THEME_DARK);
	h_eq_i64("transparence reduite", s.reduced, 1);
	h_eq_i64("clavier qwerty", s.layout, SET_LAYOUT_QWERTY);
	h_eq_i64("hauteur 1080", set_mode_get(s.mode).h, 1080);
}

static void	parse_tolerant(void)
{
	static const char	txt[] = "theme=violet\r\ninconnue=1\nsans egal\n"
		"clavier=\n=azerty\ntheme=auto\r\nclavier=qwerty";
	t_settings			s;

	h_eq_i64("seules 2 cles valides", set_parse(txt, sizeof(txt) - 1, &s), 2);
	h_eq_i64("derniere valeur valide gagne", s.theme, SET_THEME_AUTO);
	h_eq_i64("fin sans saut de ligne", s.layout, SET_LAYOUT_QWERTY);
	h_eq_i64("cle absente = defaut", s.reduced, 0);
}

static void	parse_limits(void)
{
	char		big[SET_FILE_MAX + 1];
	t_settings	s;

	memset(big, 'a', sizeof(big));
	h_eq_i64("vide", set_parse("", 0, &s), 0);
	h_eq_i64("NULL refuse", set_parse(NULL, 4, &s), E_INVAL);
	h_eq_i64("sortie NULL refuse", set_parse("", 0, NULL), E_INVAL);
	h_eq_i64("exactement SET_FILE_MAX accepte",
		set_parse(big, SET_FILE_MAX, &s), 0);
	h_eq_i64("SET_FILE_MAX + 1 refuse", set_parse(big, SET_FILE_MAX + 1, &s),
		E_RANGE);
}

static void	roundtrip(void)
{
	t_settings	a;
	t_settings	b;
	char		out[SET_FILE_MAX];
	int			n;
	uint32_t	t;

	t = 0;
	while (t <= SET_THEME_AUTO)
	{
		a = (t_settings){t, t & 1, t & 1, t + 1};
		n = set_format(&a, out, sizeof(out));
		h_true(n > 0, "format reussi");
		h_eq_i64("4 cles relues", set_parse(out, (size_t)n, &b), 4);
		h_true(memcmp(&a, &b, sizeof(a)) == 0, "aller-retour identique");
		t++;
	}
}

static void	format_limits(void)
{
	t_settings	s;
	char		out[8];

	set_default(&s);
	h_eq_i64("tampon trop court", set_format(&s, out, sizeof(out)), E_RANGE);
	h_eq_i64("tampon nul", set_format(&s, out, 0), E_INVAL);
	s.theme = 3;
	h_eq_i64("theme hors borne", set_format(&s, out, sizeof(out)), E_INVAL);
	set_default(&s);
	s.mode = SET_MODES;
	h_eq_i64("mode hors borne", set_format(&s, out, sizeof(out)), E_INVAL);
}

static void	auto_hours(void)
{
	t_settings	s;

	set_default(&s);
	s.theme = SET_THEME_AUTO;
	h_true(!set_dark_at(&s, 7), "7 h : clair (limite)");
	h_true(set_dark_at(&s, 6), "6 h : sombre");
	h_true(!set_dark_at(&s, 18), "18 h : clair");
	h_true(set_dark_at(&s, 19), "19 h : sombre (limite)");
	h_true(set_dark_at(&s, 0), "minuit : sombre");
	s.theme = SET_THEME_DARK;
	h_true(set_dark_at(&s, 12), "sombre force");
	s.theme = SET_THEME_LIGHT;
	h_true(!set_dark_at(&s, 23), "clair force");
}

int	main(void)
{
	h_begin("e12/settings");
	h_run("e12-set-defaut", defaults);
	h_run("e12-set-parse-valide", parse_valid);
	h_run("e12-set-parse-tolerant", parse_tolerant);
	h_run("e12-set-parse-limites", parse_limits);
	h_run("e12-set-aller-retour", roundtrip);
	h_run("e12-set-format-limites", format_limits);
	h_run("e12-set-heures-auto", auto_hours);
	return (h_end());
}
