#include <string.h>
#include "a07_fake.h"
#include "harness.h"
#include "proc_int.h"
#include "velum/boot.h"

static uint32_t	cut(const char *line, char *out, uint32_t cap)
{
	memset(out, 'x', BOOT_ARGS_CAP);
	return (boot_args_init(line, out, cap));
}

static void	filtre(void)
{
	char	out[BOOT_ARGS_CAP];

	h_eq_i64("ligne absente", boot_args_init(NULL, out, sizeof(out)), 0);
	h_eq_i64("tampon absent", boot_args_init("init.a", NULL, 8), 0);
	h_eq_i64("ligne vide", cut("", out, sizeof(out)), 0);
	h_eq_i64("espaces seuls", cut("    ", out, sizeof(out)), 0);
	h_eq_i64("aucun mot init.", cut("quiet selftest exit", out, 256), 0);
	h_eq_i64("init= n'est pas transmis",
		cut("quiet init=/system/bin/init", out, sizeof(out)), 0);
	h_eq_i64("prefixe init. seul refuse", cut("init. quiet", out, 256), 0);
	h_eq_i64("init. au milieu d'un mot refuse",
		cut("xinit.a  quiet", out, sizeof(out)), 0);
	h_eq_i64("un mot : longueur avec NUL",
		cut("quiet init.mort-winsrv", out, sizeof(out)), 17);
	h_eq_i64("un mot : contenu", memcmp(out, "init.mort-winsrv", 17), 0);
	h_eq_i64("espaces multiples : deux mots",
		cut("  init.a   quiet   init.bc  ", out, sizeof(out)), 15);
	h_eq_i64("deux mots : contenu", memcmp(out, "init.a\0init.bc", 15), 0);
	h_eq_i64("format accepte par args_count", args_count(out, 15), 2);
	h_eq_i64("octet de controle refuse", cut("init.a\tb", out, 256), 0);
	h_eq_i64("octet hors ASCII refuse", cut("init.\xc3\xa9", out, 256), 0);
}

static void	longueurs(void)
{
	char	line[BOOT_CMDLINE_MAX];
	char	out[BOOT_ARGS_CAP];

	memset(line, 'a', sizeof(line));
	memcpy(line, "init.", 5);
	line[BOOT_ARG_MAX] = '\0';
	h_eq_i64("mot de longueur maximale accepte",
		cut(line, out, sizeof(out)), BOOT_ARG_MAX + 1);
	line[BOOT_ARG_MAX] = 'a';
	line[BOOT_ARG_MAX + 1] = '\0';
	h_eq_i64("mot trop long d'un octet refuse entier",
		cut(line, out, sizeof(out)), 0);
	line[BOOT_ARG_MAX + 1] = ' ';
	memcpy(line + BOOT_ARG_MAX + 2, "init.b", 7);
	h_eq_i64("le mot suivant un mot trop long passe",
		cut(line, out, sizeof(out)), 7);
	memset(line, 'a', sizeof(line));
	memcpy(line, "init.", 5);
	h_eq_i64("ligne sans NUL : lecture bornee, mot refuse",
		cut(line, out, sizeof(out)), 0);
}

static void	capacite(void)
{
	char	out[BOOT_ARGS_CAP];
	int		n;

	h_eq_i64("capacite nulle", cut("init.a", out, 0), 0);
	h_eq_i64("capacite sans place pour le NUL", cut("init.a", out, 6), 0);
	h_eq_i64("rien ecrit hors capacite", out[0], 'x');
	h_eq_i64("capacite exacte", cut("init.a", out, 7), 7);
	h_eq_i64("octet suivant intact", out[7], 'x');
	h_eq_i64("second mot sans place : ignore",
		cut("init.a init.bcd", out, 10), 7);
	h_eq_i64("pas de debordement apres le premier", out[7], 'x');
	n = (int)cut("init.1 init.2 init.3 init.4 init.5 init.6 init.7 init.8"
			" init.9 init.10", out, sizeof(out));
	h_eq_i64("plafond de mots : 8 gardes", n, 8 * 7);
	h_eq_i64("plafond de mots : compte", args_count(out, (uint64_t)n),
		BOOT_NARGS_MAX);
	h_eq_i64("dernier mot garde", memcmp(out + 49, "init.8", 7), 0);
}

int	main(void)
{
	h_begin("a07/arguments_de_demarrage");
	h_run("filtre", filtre);
	h_run("longueurs", longueurs);
	h_run("capacite", capacite);
	return (h_end());
}
