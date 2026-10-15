#include <stdint.h>
#include "harness.h"
#include "ref.h"
#include "velum/gfx.h"

static void	incident_sr_y_ignoree(void)
{
	t_pair	p;

	p = (t_pair){rect_make(0, 0, 7, 6), rect_make(0, 3, 7, 3), 1};
	scale_case(7, 7, 0, p);
	p = (t_pair){rect_make(1, 1, 5, 5), rect_make(2, 1, 4, 3), 1};
	scale_case(7, 7, 0, p);
	p = (t_pair){rect_make(0, 0, 7, 7), rect_make(3, 4, 2, 2), 1};
	scale_case(7, 7, 0, p);
}

static void	incident_sr_negatif(void)
{
	t_pair	p;

	p = (t_pair){rect_make(0, 0, 7, 7), rect_make(-2, -2, 6, 6), 1};
	scale_case(7, 7, 0, p);
	p = (t_pair){rect_make(0, 0, 7, 7), rect_make(4, 4, 6, 6), 1};
	scale_case(7, 7, 0, p);
}

int	main(void)
{
	h_begin("a15/smooth_regress");
	h_run("incident I1: smooth ignorait sr.y", incident_sr_y_ignoree);
	h_run("smooth: source debordant de la surface", incident_sr_negatif);
	return (h_end());
}
