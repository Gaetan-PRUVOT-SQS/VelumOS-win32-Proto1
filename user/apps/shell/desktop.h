#ifndef DESKTOP_H
# define DESKTOP_H

# include <stdint.h>
# include "velum/gfx.h"

# define DESK_ICONS 3
# define DESK_CELL_W 90
# define DESK_CELL_H 81
# define DESK_STEP_Y 99
# define DESK_MARGIN 24
# define DESK_NONE -1
# define DBL_MAX_NS 500000000ull
# define DBL_SLOP 4
# define DESK_SLOTS_MAX 64
# define DESK_DRAG_SLOP 4

typedef enum e_deskact
{
	DA_NONE = 0,
	DA_COMPUTER,
	DA_RECYCLE,
	DA_HELLO
}	t_deskact;

typedef struct s_deskicon
{
	uint32_t	action;
	uint32_t	icon;
	const char	*label;
}	t_deskicon;

typedef struct s_click
{
	int32_t		index;
	int32_t		x;
	int32_t		y;
	uint64_t	at_ns;
}	t_click;

typedef struct s_deskstate
{
	int32_t		slot[DESK_ICONS];
	uint32_t	trashed;
	uint32_t	gone;
	int32_t		press;
	int32_t		grab_dx;
	int32_t		grab_dy;
	int32_t		press_x;
	int32_t		press_y;
	int			dragging;
	int			ready;
}	t_deskstate;

const t_deskicon	*desk_icon(uint32_t index);
int					desk_grid(t_rect area, uint32_t count, t_rect *out);
int32_t				desk_rows(t_rect area);
int32_t				desk_hit(const t_rect *cells, uint32_t n, int32_t x,
						int32_t y);
int32_t				desk_move(int32_t sel, uint32_t n, int32_t rows,
						uint32_t vk);
void				dc_reset(t_click *last);
int					dc_feed(t_click *last, const t_click *now);

void				desk_state_init(t_deskstate *st);
void				desk_press_clear(t_deskstate *st);
int					desk_visible(const t_deskstate *st, uint32_t icon);
int32_t				desk_owner(const t_deskstate *st, int32_t slot,
						int32_t except);
int					desk_drop(t_deskstate *st, uint32_t icon, int32_t slot);
int					desk_apply(const t_deskstate *st, const t_rect *slots,
						int32_t n, t_rect *cells, int32_t skip);
int32_t				desk_pick(const t_deskstate *st, const t_rect *cells,
						uint32_t n, int32_t x, int32_t y);
int					desk_slots(t_rect area, t_rect *out, uint32_t max);
int32_t				desk_slot_near(const t_rect *slots, int32_t n, int32_t x,
						int32_t y);
int					desk_deletable(uint32_t icon);
int					desk_trash_put(t_deskstate *st, uint32_t icon);
uint32_t			desk_trash_count(const t_deskstate *st);
uint32_t			desk_trash_empty(t_deskstate *st);
uint32_t			desk_trash_restore(t_deskstate *st, int32_t nslots);

#endif
