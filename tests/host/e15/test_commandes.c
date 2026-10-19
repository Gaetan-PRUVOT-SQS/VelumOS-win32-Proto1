#include <string.h>
#include "harness.h"
#include "e15.h"

static t_console	g_c;
static char			g_out[8192];

static const char	*play(const char *line)
{
	scr_clear(&g_c.screen);
	run_line(&g_c, line);
	screen_text(&g_c.screen, g_out, sizeof(g_out));
	return (g_out);
}

static void	debut(void)
{
	fake_reset();
	con_init(&g_c, &g_fake_ops, NULL, 100);
}

static void	pwd_cd(void)
{
	debut();
	h_eq_str("pwd racine", play("pwd"), "/\n\n");
	play("cd docs");
	h_eq_str("cwd apres cd", g_c.cwd, "/docs");
	h_eq_str("cd ..", play("cd .."), "\n");
	h_eq_str("cwd remonte", g_c.cwd, "/");
	h_eq_str("cd absent", play("cd nope"), "cd : introuvable\n\n");
	h_eq_str("cd fichier", play("cd /docs/notes.txt"),
		"cd : n'est pas un dossier\n\n");
	h_eq_str("cwd inchange", g_c.cwd, "/");
	play("cd /system/bin");
	play("cd");
	h_eq_str("cd sans argument va a la racine", g_c.cwd, "/");
	h_eq_str("cd trop d'arguments", play("cd a b"),
		"cd : trop d'arguments\n\n");
}

static void	ls(void)
{
	debut();
	h_eq_str("ls dossiers d'abord, ordre octet",
		play("ls /docs"), "Zoo/  accents.txt  bin.dat  notes.txt\n\n");
	play("cd /docs");
	h_eq_str("ls relatif", play("ls"), "Zoo/  accents.txt  bin.dat  notes.txt\n\n");
	h_eq_str("ls racine", play("ls /"), "docs/  system/\n\n");
	h_eq_str("ls absent", play("ls /nope"), "ls : introuvable\n\n");
	h_eq_str("ls d'un fichier", play("ls /docs/notes.txt"),
		"/docs/notes.txt\n\n");
	g_c.screen.cols = 20;
	h_eq_str("retour a la ligne par entree", play("ls /docs"),
		"Zoo/  accents.txt\nbin.dat  notes.txt\n\n");
}

static void	couleurs_ls(void)
{
	const t_conrow	*r;

	debut();
	play("ls /system");
	r = scr_row(&g_c.screen, 0);
	h_eq_u64("dossier en jaune", r->color[0], CC_DIR);
	play("ls /system/bin");
	r = scr_row(&g_c.screen, 0);
	h_eq_u64("executable en vert", r->color[0], CC_EXEC);
	play("ls /docs");
	r = scr_row(&g_c.screen, 0);
	h_eq_u64("fichier en texte", r->color[r->bytes - 1], CC_TEXT);
}

static void	cat(void)
{
	debut();
	h_eq_str("cat", play("cat /docs/notes.txt"),
		"ligne un\nligne deux\n\n");
	h_eq_str("cat sans fin de ligne", play("cat /docs/accents.txt"),
		"\xc3\xa9t\xc3\xa9\n\n");
	h_eq_str("cat octet invalide et tabulation",
		play("cat /docs/bin.dat"), "a?b    c\n\n");
	h_eq_str("cat dossier", play("cat /docs"), "cat : est un dossier\n\n");
	h_eq_str("cat absent", play("cat zzz"), "cat : introuvable\n\n");
	h_eq_str("cat sans argument", play("cat"),
		"cat : usage : cat fichier\n\n");
}

static void	echo_clear_help(void)
{
	debut();
	h_eq_str("echo", play("echo a \"b  c\""), "a b  c\n\n");
	h_eq_str("echo vide", play("echo"), "\n\n");
	h_eq_str("guillemet ouvert", play("echo \"a"),
		"console : guillemet non fermé\n\n");
	run_line(&g_c, "echo a");
	run_line(&g_c, "clear");
	h_eq_u64("clear vide l'ecran", scr_count(&g_c.screen), 1);
	h_eq_u64("clear sans ligne vide", scr_row(&g_c.screen, 0)->bytes, 0);
	play("help");
	h_true(strstr(g_out, "ls      liste un dossier") != NULL, "aide : ls");
	h_true(strstr(g_out, "uptime  ") != NULL, "aide : uptime");
	h_true(strstr(g_out, "/system/bin") != NULL, "aide : programmes");
	h_eq_i64("exit", (run_line(&g_c, "exit"), g_c.quit), 1);
}

static void	systeme(void)
{
	debut();
	h_eq_str("uptime comme la maquette", play("uptime"),
		"en marche depuis 14 s \xc2\xb7 3 processus \xc2\xb7 m\xc3\xa9moire "
		"41 / 128 Mo\n\n");
	play("mem");
	h_true(strstr(g_out, "total" "            131072 Kio") != NULL, "mem : total");
	h_true(strstr(g_out, "libre" "             89088 Kio") != NULL, "mem : libre");
	h_true(strstr(g_out, "utilis\xc3\xa9" "e") != NULL, "mem : utilisee");
	h_true(strstr(g_out, "tas noyau" "          3072 Kio") != NULL, "mem : tas");
	play("ps");
	h_true(strstr(g_out, "    1     0  actif   " "      64 Kio  init") != NULL,
		"ps : init");
	h_true(strstr(g_out, "    7     1  actif   " "    2048 Kio  winsrv") != NULL,
		"ps : winsrv");
	h_true(strstr(g_out, "   42     7  zombie  " "       0 Kio  hello") != NULL,
		"ps : zombie");
}

static void	kill(void)
{
	debut();
	h_eq_str("kill", play("kill 42"), "signal envoy\xc3\xa9\n\n");
	h_eq_u64("pid transmis", g_fake_killed, 42);
	h_eq_str("kill inconnu", play("kill 7"),
		"kill : processus inconnu de la console\n\n");
	h_eq_str("kill texte", play("kill abc"), "kill : pid invalide\n\n");
	h_eq_str("kill vide", play("kill \"\""), "kill : pid invalide\n\n");
	h_eq_str("kill trop grand", play("kill 99999999999"),
		"kill : pid invalide\n\n");
	h_eq_str("kill negatif", play("kill -1"), "kill : pid invalide\n\n");
	h_eq_str("kill sans pid", play("kill"), "kill : usage : kill pid\n\n");
}

static void	programmes(void)
{
	debut();
	h_eq_str("nom nu", play("hello"), "hello lanc\xc3\xa9 (pid 42)\n\n");
	h_eq_str("chemin lance", g_fake_spawn_path, "/system/bin/hello");
	h_eq_u64("une seule fois", g_fake_spawned, 1);
	play("hello bonjour");
	h_eq_str("argument transmis", g_fake_spawn_arg, "bonjour");
	play("cd /system/bin");
	play("./hello");
	h_eq_u64("chemin relatif", g_fake_spawned, 3);
	fake_reset();
	h_eq_str("inconnu", play("inconnu"), "inconnu : commande introuvable\n\n");
	h_eq_str("dossier n'est pas une commande", play("/docs"),
		"/docs : commande introuvable\n\n");
	h_eq_str("echappement du dossier des binaires", play(".."),
		".. : commande introuvable\n\n");
	h_eq_u64("rien lance", g_fake_spawned, 0);
}

static void	invite_et_validation(void)
{
	t_conseg	seg[CON_SEGS];
	char		typed[] = "pwd";

	debut();
	h_eq_i64("sept segments", con_prompt_segs(&g_c, seg), CON_SEGS);
	h_eq_str("utilisateur", seg[0].text, "utilisateur");
	h_eq_u64("couleur utilisateur", seg[0].color, CC_USER);
	h_eq_str("hote", seg[1].text, "@velum");
	h_eq_u64("couleur hote", seg[1].color, CC_AT);
	h_eq_u64("couleur chemin", seg[3].color, CC_PATH);
	scr_clear(&g_c.screen);
	con_echo(&g_c, typed);
	h_eq_str("ligne d'invite", scr_row(&g_c.screen, 0)->text,
		"utilisateur@velum / > pwd");
	line_insert(&g_c.line, "echo ok", 7);
	con_submit(&g_c);
	screen_text(&g_c.screen, g_out, sizeof(g_out));
	h_true(strstr(g_out, "utilisateur@velum / > echo ok\nok\n\n") != NULL,
		"soumission : invite, sortie, ligne vide");
	h_eq_u64("ligne videe", g_c.line.len, 0);
}

static void	hors_systeme(void)
{
	debut();
	h_eq_str("fichier hors binaires", play("/docs/notes.txt"),
		"/docs/notes.txt : commande introuvable\n\n");
	h_eq_str("texte hors binaires", play("/system/etc/os-release"),
		"/system/etc/os-release : commande introuvable\n\n");
	h_eq_str("remontee hors binaires", play("/system/bin/../etc/os-release"),
		"/system/bin/../etc/os-release : commande introuvable\n\n");
	h_eq_str("system puis docs", play("/system/../docs/notes.txt"),
		"/system/../docs/notes.txt : commande introuvable\n\n");
	h_eq_str("prefixe voisin", play("/system/binx/hello"),
		"/system/binx/hello : commande introuvable\n\n");
	h_eq_u64("aucun programme lance", g_fake_spawned, 0);
	h_eq_str("binaire explicite", play("/system/bin/hello"),
		"/system/bin/hello lanc\xc3\xa9 (pid 42)\n\n");
	h_eq_str("chemin normalise du binaire", g_fake_spawn_path,
		"/system/bin/hello");
	fake_reset();
	h_eq_str("slashes et point inutiles", play("//system/bin/./hello"),
		"//system/bin/./hello lanc\xc3\xa9 (pid 42)\n\n");
	h_eq_str("toujours le binaire", g_fake_spawn_path, "/system/bin/hello");
	debut();
	h_eq_str("argument qui sort", play("hello /docs/notes.txt"),
		"hello : acc\xc3\xa8s refus\xc3\xa9\n\n");
	h_eq_str("guillemets ne font pas passer",
		play("hello \"/docs/notes.txt\""),
		"hello : acc\xc3\xa8s refus\xc3\xa9\n\n");
	h_eq_str("prefixe /systemd", play("hello /systemd"),
		"hello : acc\xc3\xa8s refus\xc3\xa9\n\n");
	h_eq_str("argument system puis docs",
		play("hello /system/../docs/notes.txt"),
		"hello : acc\xc3\xa8s refus\xc3\xa9\n\n");
	h_eq_u64("arguments refuses", g_fake_spawned, 0);
	h_eq_str("octet de controle", play("hello \"a\nb\""),
		"hello : argument invalide\n\n");
	play("cd /system/bin");
	h_eq_str("argument qui remonte hors systeme",
		play("hello ../../docs/notes.txt"),
		"hello : acc\xc3\xa8s refus\xc3\xa9\n\n");
	h_eq_u64("toujours rien", g_fake_spawned, 0);
	h_eq_str("argument ramene sous systeme", play("hello ../etc/os-release"),
		"hello lanc\xc3\xa9 (pid 42)\n\n");
	h_eq_str("argument absolu transmis", g_fake_spawn_arg,
		"/system/etc/os-release");
	debut();
	h_eq_str("echo cite un chemin hors systeme", play("echo /docs/notes.txt"),
		"/docs/notes.txt\n\n");
	h_eq_str("cd refuse un controle", play("cd /docs/a\nb"),
		"cd : argument invalide\n\n");
	h_eq_str("cd reste a la racine", g_c.cwd, "/");
}

static void	trop_d_arguments(void)
{
	debut();
	h_eq_str("seize arguments max", play("a b c d e f g h i j k l m n o p q"),
		"console : trop d'arguments\n\n");
	h_eq_str("ligne vide sans effet", play("   "), "");
}

int	main(void)
{
	h_begin("e15_commandes");
	h_run("pwd_cd", pwd_cd);
	h_run("ls", ls);
	h_run("couleurs_ls", couleurs_ls);
	h_run("cat", cat);
	h_run("echo_clear_help", echo_clear_help);
	h_run("systeme", systeme);
	h_run("kill", kill);
	h_run("programmes", programmes);
	h_run("hors_systeme", hors_systeme);
	h_run("invite_et_validation", invite_et_validation);
	h_run("trop_d_arguments", trop_d_arguments);
	return (h_end());
}
