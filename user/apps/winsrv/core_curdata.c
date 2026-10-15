#include "ws_core.h"

static const char	g_cur_arrow[] = "X           "
	"XX          "
	"X.X         "
	"X..X        "
	"X...X       "
	"X....X      "
	"X.....X     "
	"X......X    "
	"X.......X   "
	"X........X  "
	"X.........X "
	"X......XXXXX"
	"X...X..X    "
	"X..XX..X    "
	"X.X  X..X   "
	"XX   X..X   "
	"X     X..X  "
	"      X..X  "
	"       XX   ";

static const char	g_cur_ibeam[] = "XXX XXX"
	"   X   "
	"   X   "
	"   X   "
	"   X   "
	"   X   "
	"   X   "
	"   X   "
	"   X   "
	"   X   "
	"   X   "
	"   X   "
	"   X   "
	"   X   "
	"   X   "
	"XXX XXX";

static const char	g_cur_wait[] = "XXXXXXXXXXX"
	"X.........X"
	"XXXXXXXXXXX"
	" X.......X "
	" X.......X "
	" X.X.X.X.X "
	"  X.X.X.X  "
	"   X.X.X   "
	"    X.X    "
	"    X.X    "
	"   X...X   "
	"  X.....X  "
	" X...X...X "
	" X..XXX..X "
	" X.XXXXX.X "
	"XXXXXXXXXXX"
	"X.........X"
	"XXXXXXXXXXX";

static const char	g_cur_ns[] = "   X   "
	"  X.X  "
	" X...X "
	"X.....X"
	"XXX.XXX"
	"  X.X  "
	"  X.X  "
	"  X.X  "
	"  X.X  "
	"  X.X  "
	"XXX.XXX"
	"X.....X"
	" X...X "
	"  X.X  "
	"   X   ";

static const char	g_cur_we[] = "   XX     XX   "
	"  X.X     X.X  "
	" X..XXXXXXX..X "
	"X.............X"
	" X..XXXXXXX..X "
	"  X.X     X.X  "
	"   XX     XX   ";

static const char	g_cur_nwse[] = "XXXXX      "
	"X...X      "
	"X..X       "
	"X.X.X      "
	"XX X.X     "
	"    X.X    "
	"     X.X XX"
	"      X.X.X"
	"       X..X"
	"      X...X"
	"      XXXXX";

static const char	g_cur_nesw[] = "      XXXXX"
	"      X...X"
	"       X..X"
	"      X.X.X"
	"     X.X XX"
	"    X.X    "
	"XX X.X     "
	"X.X.X      "
	"X..X       "
	"X...X      "
	"XXXXX      ";

static const char	g_cur_hand[] = "    XX        "
	"   X..X       "
	"   X..X       "
	"   X..X       "
	"   X..XXX     "
	"   X..X..XX   "
	"   X..X..X.XX "
	"XX X..X..X.X.X"
	"X..X.........X"
	"X............X"
	" X...........X"
	"  X..........X"
	"  X.........X "
	"   X........X "
	"    X......X  "
	"    XXXXXXXX  ";

void	wcur_get(uint32_t shape, t_wcurdef *out)
{
	static const t_wcurdef	defs[] = {{g_cur_arrow, 12, 19, 0, 0},
	{g_cur_ibeam, 7, 16, 3, 8},
	{g_cur_wait, 11, 18, 5, 9}, {g_cur_ns, 7, 15, 3, 7},
	{g_cur_we, 15, 7, 7, 3}, {g_cur_nwse, 11, 11, 5, 5},
	{g_cur_nesw, 11, 11, 5, 5}, {g_cur_hand, 14, 16, 5, 0}};

	if (shape > CUR_HAND)
		shape = CUR_ARROW;
	*out = defs[shape];
}
