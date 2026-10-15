#include <stdio.h>
#include "harness.h"
#include "fake.h"

static const int	g_mut[13][3][3] = {
{{0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
{{2, CTL_ENABLED, 1}, {0, 0, 0}, {0, 0, 0}},
{{3, CTL_VISIBLE, 1}, {0, 0, 0}, {0, 0, 0}},
{{5, CTL_VISIBLE, 0}, {0, 0, 0}, {0, 0, 0}},
{{5, CTL_ENABLED, 0}, {0, 0, 0}, {0, 0, 0}},
{{6, CTL_FOCUSABLE, 0}, {0, 0, 0}, {0, 0, 0}},
{{9, CTL_CHECKED, 1}, {0, 0, 0}, {0, 0, 0}},
{{9, CTL_CHECKED, 1}, {9, CTL_ENABLED, 0}, {0, 0, 0}},
{{8, CTL_ENABLED, 0}, {0, 0, 0}, {0, 0, 0}},
{{8, CTL_ENABLED, 0}, {9, CTL_ENABLED, 0}, {10, CTL_ENABLED, 0}},
{{7, CTL_VISIBLE, 0}, {0, 0, 0}, {0, 0, 0}},
{{1, CTL_ENABLED, 0}, {0, 0, 0}, {0, 0, 0}},
{{1, CTL_FOCUSABLE, 0}, {0, 0, 0}, {0, 0, 0}}};
static const int	g_exp[13][6] = {
{1, 6, 8, 11, -1, -1}, {1, 2, 6, 8, 11, -1}, {1, 3, 6, 8, 11, -1},
{1, 8, 11, -1, -1, -1}, {1, 8, 11, -1, -1, -1}, {1, 8, 11, -1, -1, -1},
{1, 6, 9, 11, -1, -1}, {1, 6, 8, 11, -1, -1}, {1, 6, 9, 11, -1, -1},
{1, 6, 11, -1, -1, -1}, {1, 6, 11, -1, -1, -1}, {6, 8, 11, -1, -1, -1},
{6, 8, 11, -1, -1, -1}};

static void	check_row(int row, const int *got, int n, bool rev)
{
	int		len;
	int		i;
	char	name[48];

	len = 0;
	while (g_exp[row][len] >= 0)
		len++;
	snprintf(name, sizeof(name), "ligne %d : longueur", row);
	h_eq_i64(name, n, len);
	i = 0;
	while (i < n && i < len)
	{
		snprintf(name, sizeof(name), "ligne %d : rang %d", row, i);
		if (rev)
			h_eq_i64(name, got[i], g_exp[row][len - 1 - i]);
		else
			h_eq_i64(name, got[i], g_exp[row][i]);
		i++;
	}
}

static void	run(bool forward)
{
	t_fakeui	u;
	int			row;
	int			got[8];

	row = 0;
	while (row < 13)
	{
		fake_begin(&u, 300, 200);
		fake_fscene(&u);
		fake_apply(&u, g_mut[row]);
		check_row(row, got, fake_tabs(&u, got, forward), !forward);
		fake_done(&u);
		row++;
	}
}

static void	forward_table(void)
{
	run(true);
}

static void	backward_table(void)
{
	run(false);
}

int	main(void)
{
	h_begin("a19/focus");
	h_run("Tab : table de decision (13 regles)", forward_table);
	h_run("Maj+Tab : ordre inverse de la table", backward_table);
	return (h_end());
}
