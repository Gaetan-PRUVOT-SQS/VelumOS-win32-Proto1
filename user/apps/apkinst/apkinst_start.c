#include "velum/err.h"
#include "velum/libk.h"
#include "../common/platform.h"
#include "apkinst.h"

int	main(int argc, char **argv)
{
	static t_pmentry	entry;
	int					r;

	if (argc < 2)
	{
		os_log("apkinst: refusé : chemin de l'APK manquant");
		return (1);
	}
	memset(&entry, 0, sizeof(entry));
	r = apkinst_install(argv[1], &entry);
	apkinst_report(r, &entry);
	return (r < 0);
}
