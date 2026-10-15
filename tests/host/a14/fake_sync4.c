#include "fake_sync.h"
#include "stdlib.h"

void	fs_h1(void)
{
	fs_h_push(1);
}

void	fs_h2(void)
{
	fs_h_push(2);
}

void	fs_h3(void)
{
	fs_h_push(3);
}

void	fs_h_reg(void)
{
	fs_h_push(9);
	atexit(fs_h1);
}

void	fs_h_noop(void)
{
	fs_h_push(5);
}
