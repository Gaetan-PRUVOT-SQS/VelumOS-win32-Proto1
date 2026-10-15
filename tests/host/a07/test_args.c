#include <stdlib.h>
#include <string.h>
#include "a07_fake.h"
#include "harness.h"
#include "velum/err.h"
#include "velum/proc.h"

static void	args_partitions(void)
{
	char	*big;

	h_eq_i64("longueur 0", args_count(NULL, 0), 0);
	h_eq_i64("pointeur nul", args_count(NULL, 4), E_INVAL);
	h_eq_i64("non termine", args_count("abc", 3), E_INVAL);
	h_eq_i64("un argument", args_count("abc", 4), 1);
	h_eq_i64("argument vide", args_count("", 1), 1);
	big = calloc(PROC_ARGS_MAX + 1, 1);
	h_eq_i64("limite 32 Kio de NUL", args_count(big, PROC_ARGS_MAX),
		E_INVAL);
	free(big);
	big = args_block(PROC_NARGS_MAX, 128);
	h_eq_i64("limite 256 args sur 32 Kio", args_count(big, PROC_ARGS_MAX),
		PROC_NARGS_MAX);
	big[PROC_ARGS_MAX] = '\0';
	h_eq_i64("32 Kio + 1", args_count(big, PROC_ARGS_MAX + 1), E_INVAL);
	memset(big, '\0', 257);
	h_eq_i64("257 args", args_count(big, 257), E_INVAL);
	h_eq_i64("limite 256 args", args_count(big, 256), 256);
	free(big);
}

static void	path_decision(void)
{
	char	p[300];

	h_eq_i64("chemin racine", path_check("/", 1), 0);
	h_eq_i64("MC/DC pointeur nul", path_check(NULL, 1), E_INVAL);
	h_eq_i64("MC/DC longueur 0", path_check("/", 0), E_INVAL);
	h_eq_i64("MC/DC chemin relatif", path_check("a/b", 3), E_INVAL);
	memset(p, 'a', sizeof(p));
	p[0] = '/';
	h_eq_i64("limite 255 octets", path_check(p, 255), 0);
	h_eq_i64("MC/DC 256 octets", path_check(p, 256), E_INVAL);
	h_eq_i64("NUL au milieu", path_check("/a\0b", 4), E_INVAL);
	h_eq_i64("caractere de controle", path_check("/a\nb", 4), E_INVAL);
	h_eq_i64("DEL", path_check("/a\177", 3), E_INVAL);
	h_eq_i64("echappement ANSI", path_check("/\033[2J", 5), E_INVAL);
}

static void	path_utf8(void)
{
	h_eq_i64("UTF-8 2 octets", path_check("/\303\251", 3), 0);
	h_eq_i64("UTF-8 3 octets", path_check("/\342\202\254", 4), 0);
	h_eq_i64("UTF-8 4 octets", path_check("/\360\237\230\200", 5), 0);
	h_eq_i64("limite U+10FFFF", path_check("/\364\217\277\277", 5), 0);
	h_eq_i64("au-dela de U+10FFFF", path_check("/\364\220\200\200", 5),
		E_INVAL);
	h_eq_i64("sur-long 2 octets", path_check("/\300\257", 3), E_INVAL);
	h_eq_i64("sur-long 3 octets", path_check("/\340\200\257", 4), E_INVAL);
	h_eq_i64("sur-long 4 octets", path_check("/\360\200\200\257", 5),
		E_INVAL);
	h_eq_i64("demi-codet", path_check("/\355\240\200", 4), E_INVAL);
	h_eq_i64("controle C1", path_check("/\302\205", 3), E_INVAL);
	h_eq_i64("limite U+00A0", path_check("/\302\240", 3), 0);
	h_eq_i64("sequence tronquee", path_check("/\342\202", 3), E_INVAL);
	h_eq_i64("continuation seule", path_check("/\200", 2), E_INVAL);
	h_eq_i64("octet 0xff", path_check("/\377", 2), E_INVAL);
	h_eq_i64("continuation fausse", path_check("/\303\050", 3), E_INVAL);
}

static void	names(void)
{
	char	n[PROC_NAME_LEN];

	proc_name_from_path(n, "/system/bin/init", 16);
	h_eq_str("nom de base", n, "init");
	proc_name_from_path(n, "/", 1);
	h_eq_str("racine sans nom", n, "?");
	proc_name_from_path(n, "/x/0123456789012345678901234567890123456789",
		43);
	h_eq_str("nom tronque a 31", n, "0123456789012345678901234567890");
	proc_name_from_path(n, "/a\tb", 4);
	h_eq_str("controle remplace", n, "a?b");
	proc_name_from_path(n, "/sys/le\0reste", 13);
	h_eq_str("arret au NUL", n, "le");
}

int	main(void)
{
	h_begin("a07/chemins_et_arguments");
	h_run("arguments", args_partitions);
	h_run("decision du chemin", path_decision);
	h_run("UTF-8", path_utf8);
	h_run("noms de processus", names);
	return (h_end());
}
