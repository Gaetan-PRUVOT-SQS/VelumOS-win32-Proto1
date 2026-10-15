#include "cases.h"
#include "kfix.h"

static void	same_line(const t_bsod_wrap *w, const t_wrapcase *c, int32_t i)
{
	size_t	len;

	len = strlen(c->line[i]);
	h_true(w->lines[i].len == (int32_t)len
		&& !memcmp(w->lines[i].s, c->line[i], len), "contenu de la ligne");
}

static void	run_case(const t_wrapcase *c)
{
	t_bsod_wrap	w;
	int32_t		i;

	g_h.name = c->name;
	fake_font_reset();
	w.text = c->text;
	w.cols = c->cols;
	w.max = c->max;
	bsod_wrap(&w);
	h_eq_i64("nombre de lignes", w.count, c->count);
	h_true(w.truncated == c->trunc, "drapeau de troncature");
	h_eq_i64("proprietes du decoupage", fake_wrapck(&w, c->text, c->cols), 0);
	i = 0;
	while (i < w.count && i < c->count)
		same_line(&w, c, i++);
}

static void	table_all(void)
{
	int	i;

	i = 0;
	while (g_wrap_cases[i].name)
		run_case(&g_wrap_cases[i++]);
}

static void	max_is_capped(void)
{
	t_bsod_wrap	w;

	w.text = "a\na\na\na\na\na\na\na\na\na\na\na\na\na\na\na\na\na\na\na";
	w.cols = 5;
	w.max = 100;
	bsod_wrap(&w);
	h_eq_i64("au plus BSOD_LINES_MAX lignes", w.count, BSOD_LINES_MAX);
	h_true(w.truncated, "reste du texte signale");
}

int	main(void)
{
	h_begin("a13/bsod_table");
	h_run("decoupage table de cas", table_all);
	h_run("decoupage nombre de lignes plafonne", max_is_capped);
	return (h_end());
}
