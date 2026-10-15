#include "vtest.h"

int	main(int argc, char **argv)
{
	v_logf(V_LOG_INFO, "VTEST start argc=%d argv0=%s", argc, argv[0]);
	vtest_mem();
	vtest_str();
	vtest_sys();
	vtest_files();
	vtest_thr();
	return (vtest_report());
}
