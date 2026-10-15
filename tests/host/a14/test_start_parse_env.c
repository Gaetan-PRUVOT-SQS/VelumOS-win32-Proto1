#include <signal.h>
#include <stdint.h>
#include <sys/wait.h>
#include <unistd.h>
#include "fake_f4.h"
#include "harness.h"
#include "velum/err.h"

static void	env_counts(void)
{
	t_f4_stack	s;
	t_startinfo	info;

	f4_reset(&s);
	f4_head(&s, 1, 0);
	f4_aux_full(&s);
	h_eq_i64("envp NULL seul", f4_parse(&s, s.n, &info), START_OK);
	h_eq_u64("envc 0", info.envc, 0);
	f4_reset(&s);
	f4_head(&s, 1, 1);
	f4_aux_full(&s);
	h_eq_i64("une variable", f4_parse(&s, s.n, &info), START_OK);
	h_eq_u64("envc 1", info.envc, 1);
	f4_reset(&s);
	f4_head(&s, 1, 256);
	f4_aux_full(&s);
	h_eq_i64("256 variables", f4_parse(&s, s.n, &info), START_OK);
	h_eq_u64("envc 256", info.envc, 256);
}

static void	env_too_many(void)
{
	t_f4_stack	s;
	t_startinfo	info;

	f4_reset(&s);
	f4_head(&s, 1, 257);
	f4_aux_full(&s);
	h_eq_i64("257 variables", f4_parse(&s, s.n, &info), E_INVAL);
	f4_reset(&s);
	f4_head(&s, 0, 600);
	f4_aux_full(&s);
	h_eq_i64("600 variables", f4_parse(&s, s.n, &info), E_INVAL);
	f4_reset(&s);
	f4_head(&s, 0, 256);
	h_eq_i64("256 variables et fenetre finie sur le NULL", f4_parse(&s, s.n,
			&info), START_TRUNCATED);
	h_eq_i64("fenetre coupee avant le NULL de env", f4_parse(&s, s.n - 1,
			&info), E_PROTO);
}

static void	env_pointers(void)
{
	t_f4_stack		s;
	t_startinfo		info;
	const uint64_t	*buf;

	f4_reset(&s);
	f4_head(&s, 3, 2);
	f4_aux_full(&s);
	buf = f4_open(&s, s.n);
	h_eq_i64("pile avec 3 arguments et 2 variables",
		start_parse(buf, s.n, &info), START_OK);
	h_true((const uint64_t *)info.argv == buf + 1, "argv dans la pile");
	h_true((const uint64_t *)info.envp == buf + 5, "envp dans la pile");
	h_eq_u64("argc", info.argc, 3);
	h_eq_u64("envc", info.envc, 2);
	h_eq_u64("argv[0]", (uint64_t)(uintptr_t)info.argv[0], F4_PTR_BASE);
	h_eq_u64("envp[1]", (uint64_t)(uintptr_t)info.envp[1],
		F4_PTR_BASE + 0x8000 + 16);
	f4_close(buf, s.n);
}

static void	guard_detects_overread(void)
{
	t_f4_stack	s;
	t_startinfo	info;
	pid_t		pid;
	int			status;

	f4_reset(&s);
	f4_head(&s, 1, 0);
	f4_pairs(&s, 10, 0x1234);
	pid = fork();
	if (pid == 0)
	{
		signal(SIGSEGV, SIG_DFL);
		start_parse(f4_open(&s, 6), 40, &info);
		_exit(0);
	}
	waitpid(pid, &status, 0);
	h_true(WIFSIGNALED(status), "fenetre annoncee trop grande : SIGSEGV");
	h_eq_i64("signal recu", WTERMSIG(status), SIGSEGV);
}

int	main(void)
{
	h_begin("a14/start_parse_env");
	h_run("start_parse/valeur limite : nombre de variables", env_counts);
	h_run("start_parse/valeur limite : trop de variables", env_too_many);
	h_run("start_parse/exigence : pointeurs argv et envp", env_pointers);
	h_run("garde/supposition d'erreur : une lecture hors fenetre est vue",
		guard_detects_overread);
	return (h_end());
}
