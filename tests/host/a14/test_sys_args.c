#include "fake_f1_buf.h"
#include "fake_sys.h"
#include "harness.h"
#include "string.h"
#include "velum/velum.h"

static void	pack_basic(void)
{
	const char	*argv[4];
	const char	*none[1];
	char		*buf;

	argv[0] = "ab";
	argv[1] = "";
	argv[2] = "c";
	argv[3] = NULL;
	none[0] = NULL;
	buf = fake_guarded(6);
	h_eq_i64("argv vide", v_args_pack(NULL, 0, none), 0);
	h_eq_i64("taille sans tampon", v_args_pack(NULL, 0, argv), 6);
	h_eq_i64("ecriture", v_args_pack(buf, 6, argv), 6);
	h_true(!memcmp(buf, "ab\0\0c\0", 6), "contenu exact avec argument vide");
	h_eq_i64("argv NULL", v_args_pack(buf, 6, NULL), E_FAULT);
	fake_guarded_free(buf, 6);
}

static void	pack_count(void)
{
	const char	*argv[260];

	f1_fill_argv(argv, 255, "x");
	h_eq_i64("255 arguments", v_args_pack(NULL, 0, argv), 510);
	f1_fill_argv(argv, V_ARGC_MAX, "x");
	h_eq_i64("256 arguments", v_args_pack(NULL, 0, argv), 512);
	f1_fill_argv(argv, V_ARGC_MAX + 1, "x");
	h_eq_i64("257 arguments", v_args_pack(NULL, 0, argv), E_INVAL);
	f1_fill_argv(argv, 258, "x");
	h_eq_i64("258 arguments", v_args_pack(NULL, 0, argv), E_INVAL);
}

static void	pack_total(void)
{
	const char	*argv[3];
	char		*a;
	char		*b;

	a = f1_cstr(V_ARGS_MAX - 1);
	f1_fill_argv(argv, 1, a);
	h_eq_i64("total 32768", v_args_pack(NULL, 0, argv), V_ARGS_MAX);
	f1_release(a, V_ARGS_MAX);
	a = f1_unterminated(V_ARGS_MAX + 1);
	f1_fill_argv(argv, 1, a);
	h_eq_i64("total 32769 non termine", v_args_pack(NULL, 0, argv), E_INVAL);
	f1_release(a, V_ARGS_MAX + 1);
	a = f1_cstr(V_ARGS_MAX / 2);
	b = f1_cstr(V_ARGS_MAX / 2 - 1);
	argv[0] = a;
	argv[1] = b;
	argv[2] = NULL;
	h_eq_i64("deux arguments 32769", v_args_pack(NULL, 0, argv), E_INVAL);
	f1_release(a, V_ARGS_MAX / 2 + 1);
	f1_release(b, V_ARGS_MAX / 2);
}

static void	pack_cap(void)
{
	const char	*argv[3];
	char		*small;
	char		*exact;

	f1_fill_argv(argv, 2, "abc");
	argv[1] = "de";
	small = f1_unterminated(6);
	exact = f1_unterminated(7);
	h_eq_i64("cap 6 : taille requise", v_args_pack(small, 6, argv), 7);
	h_true(!memcmp(small, "aaaaaa", 6), "cap trop petit : rien d'ecrit");
	h_eq_i64("cap 0 avec tampon", v_args_pack(small, 0, argv), 7);
	h_true(!memcmp(small, "aaaaaa", 6), "cap nul : rien d'ecrit");
	h_eq_i64("cap exact 7", v_args_pack(exact, 7, argv), 7);
	h_true(!memcmp(exact, "abc\0de\0", 7), "cap exact : contenu sans debord");
	f1_release(small, 6);
	f1_release(exact, 7);
}

int	main(void)
{
	h_begin("a14/sys_args");
	h_run("v_args_pack/exigence : contenu, vide, NULL", pack_basic);
	h_run("v_args_pack/valeur limite : 255, 256, 257 arguments", pack_count);
	h_run("v_args_pack/valeur limite : total 32768 et 32769", pack_total);
	h_run("v_args_pack/partition : cap trop petit, nul, exact", pack_cap);
	return (h_end());
}
