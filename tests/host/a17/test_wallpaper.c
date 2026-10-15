#include <stdio.h>
#include <time.h>
#include "harness.h"
#include "lt_gal.h"

static uint64_t	now_ns(void)
{
	struct timespec	ts;

	clock_gettime(CLOCK_MONOTONIC, &ts);
	return ((uint64_t)ts.tv_sec * 1000000000ULL + (uint64_t)ts.tv_nsec);
}

static void	wallpaper_render(void)
{
	t_lt		t;
	uint64_t	t0;
	uint64_t	t1;

	h_true(lt_open(&t, 1024, 768) == 0, "allocation");
	t0 = now_ns();
	luna_wallpaper(&t.s, lp_rect(0, 0, 1024, 768));
	t1 = now_ns();
	printf("luna_wallpaper 1024x768 : %llu ms (hote, ASan+UBSan, -O1)\n",
		(unsigned long long)((t1 - t0) / 1000000ULL));
	lt_out_dir();
	h_true(lt_png("build/a17/papier_peint.png", &t.s, 1) == 0, "png");
	lt_close(&t);
}

int	main(void)
{
	h_begin("a17/wallpaper");
	h_run("rendu et temps", wallpaper_render);
	return (h_end());
}
