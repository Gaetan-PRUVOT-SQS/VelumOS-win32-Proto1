#include "velum/err.h"
#include "velum/libk.h"
#include "../common/tbuf.h"
#include "fichiers_model.h"

static const char	*g_units[] = {"o", "Ko", "Mo", "Go", "To"};

int	ff_size_text(char *out, size_t cap, uint64_t size)
{
	t_tbuf		b;
	uint64_t	tenths;
	uint32_t	unit;
	uint32_t	shift;

	unit = 0;
	while ((size >> (10 * (unit + 1))) > 0 && unit < 4)
		unit++;
	shift = 10 * unit;
	tenths = ((size & ((1ull << shift) - 1)) * 10) >> shift;
	tb_init(&b, out, cap);
	tb_num(&b, size >> shift, 0);
	if (unit > 0)
	{
		tb_putc(&b, ',');
		tb_num(&b, tenths, 0);
	}
	tb_putc(&b, ' ');
	tb_str(&b, g_units[unit]);
	return (tb_result(&b));
}

const char	*ff_dir_title(const char *path)
{
	const char	*slash;

	if (!path || path[0] != '/' || ff_path_is_root(path))
		return ("Ordinateur");
	slash = strrchr(path, '/');
	if (!slash || slash[1] == '\0')
		return ("Ordinateur");
	return (slash + 1);
}

int	ff_count_text(char *out, size_t cap, uint32_t count)
{
	t_tbuf	b;

	tb_init(&b, out, cap);
	tb_num(&b, count, 0);
	tb_str(&b, count > 1 ? " éléments" : " élément");
	return (tb_result(&b));
}

const char	*ff_error_text(int code)
{
	if (code == E_EXIST)
		return ("Ce nom existe déjà.");
	if (code == E_NOENT)
		return ("Élément introuvable.");
	if (code == E_NOTEMPTY)
		return ("Le dossier n'est pas vide.");
	if (code == E_INVAL)
		return ("Nom invalide.");
	if (code == E_RANGE)
		return ("Chemin trop long.");
	if (code == E_NOSPC)
		return ("Plus d'espace disponible.");
	if (code == E_ACCES || code == E_PERM)
		return ("Accès refusé, volume en lecture seule ?");
	if (code == E_NOMEM)
		return ("Mémoire insuffisante.");
	return ("Opération impossible.");
}
