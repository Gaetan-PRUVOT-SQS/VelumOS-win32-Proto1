#include "apk_int.h"

static const char	*g_apk_reasons[APKR_COUNT] = {
	"Aucun refus.",
	"Le fichier n'est pas une archive APK lisible.",
	"L'application ou l'un de ses fichiers est trop volumineux.",
	"L'archive contient un nom de fichier dangereux.",
	"L'application contient du code natif, non pris en charge.",
	"L'application contient plusieurs fichiers DEX, non pris en charge.",
	"Il manque le manifeste ou le code de l'application.",
	"La signature de l'application est absente ou invalide.",
	"L'algorithme de signature n'est pas pris en charge.",
	"L'application a plusieurs signataires, non pris en charge.",
	"Le manifeste de l'application est illisible.",
	"Le nom de paquet de l'application est invalide.",
	"L'application n'a pas d'écran principal à lancer.",
	"Le nom de l'application est introuvable ou invalide.",
	"La mémoire manque pour lire l'application."
};

const char	*apk_reason(int reason)
{
	if (reason < 0 || reason >= APKR_COUNT)
		return ("Refus inconnu.");
	return (g_apk_reasons[reason]);
}

int64_t	apk_read(const t_apk *a, const char *name, uint8_t **out)
{
	t_zipent	e;
	uint8_t		*buf;
	int64_t		n;

	if (out == NULL)
		return (E_INVAL);
	*out = NULL;
	if (a == NULL || name == NULL || a->reason != APKR_OK || !a->file.p)
		return (E_INVAL);
	if (zip_find(&a->zip, name, &e) < 0)
		return (E_NOENT);
	if (e.usize > APK_ENTRY_MAX)
		return (E_RANGE);
	buf = malloc((size_t)e.usize + 1);
	if (buf == NULL)
		return (E_NOMEM);
	n = zip_extract(&a->zip, &e, buf, e.usize);
	if (n == (int64_t)e.usize)
		*out = buf;
	else
		free(buf);
	if (n >= 0 && n != (int64_t)e.usize)
		return (E_INVAL);
	return (n);
}
