#include <stdint.h>
#include <string.h>
#include "fake_sys.h"
#include "harness.h"
#include "stdlib.h"
#include "velum/vmisc.h"

static void	call_abort(void)
{
	abort();
}

static void	log_split_lines(void)
{
	fake_reset();
	fake_kernel_on();
	h_eq_i64("retour = longueur formatee",
		v_logf(V_LOG_WARN, "a\nb\n\nc%d", 7), 7);
	h_eq_i64("3 lignes", g_fsys.log_count, 3);
	h_eq_str("ligne 1", g_fsys.log_lines[0], "a");
	h_eq_str("ligne 2", g_fsys.log_lines[1], "b");
	h_eq_str("ligne 3", g_fsys.log_lines[2], "c7");
	h_eq_u64("niveau transmis", g_fsys.log_levels[0], V_LOG_WARN);
	fake_reset();
	fake_kernel_on();
	v_logf(V_LOG_INFO, "%s", "");
	h_eq_i64("chaine vide : aucune ligne", g_fsys.log_count, 0);
}

static void	log_long_lines(void)
{
	char	big[700];

	fake_reset();
	fake_kernel_on();
	memset(big, 'x', sizeof(big));
	big[450] = '\0';
	h_eq_i64("450 octets", v_logf(V_LOG_INFO, "%s", big), 450);
	h_eq_i64("3 troncons", g_fsys.log_count, 3);
	h_eq_u64("200", strlen(g_fsys.log_lines[0]), 200);
	h_eq_u64("200", strlen(g_fsys.log_lines[1]), 200);
	h_eq_u64("50", strlen(g_fsys.log_lines[2]), 50);
	fake_reset();
	fake_kernel_on();
	big[450] = 'x';
	big[600] = '\0';
	h_eq_i64("600 octets voulus", v_logf(V_LOG_INFO, "%s", big), 600);
	h_eq_i64("tampon de 512 : 3 troncons", g_fsys.log_count, 3);
	h_eq_u64("dernier tronque a 111", strlen(g_fsys.log_lines[2]), 111);
}

static void	abort_code(void)
{
	fake_reset();
	fake_kernel_on();
	h_eq_i64("abort = 134", fake_exit_catch(call_abort), 134);
	h_eq_i64("une ligne de journal", g_fsys.log_count, 1);
	h_eq_str("message", g_fsys.log_lines[0], "abort()");
}

int	main(void)
{
	h_begin("a14/log");
	h_run("log/partition : decoupage aux fins de ligne", log_split_lines);
	h_run("log/limites : troncons de 200 et tampon de 512", log_long_lines);
	h_run("abort/exigence : code et journal", abort_code);
	return (h_end());
}
