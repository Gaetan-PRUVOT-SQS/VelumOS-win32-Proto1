#include "fake.h"

void	luna_metrics(t_lunametrics *out)
{
	out->caption_h = 30;
	out->frame_w = 4;
	out->frame_bottom = 4;
	out->btn_w = 21;
	out->btn_h = 21;
	out->taskbar_h = 30;
	out->start_w = 100;
	out->tray_w = 80;
	out->scroll_w = 17;
	out->menu_item_h = 22;
	out->icon_small = 16;
	out->icon_large = 32;
}

t_color	luna_color_text(void)
{
	return (0xff000000);
}

t_color	luna_color_window(void)
{
	return (0xffece9d8);
}

t_color	luna_color_selection(void)
{
	return (0xff316ac5);
}

void	luna_button(t_surface *s, const t_lunabtn *b)
{
	fake_log_add(s, FK_BUTTON, b->r, (int)b->state);
	fake_log_b((int)b->is_default | ((int)b->focus << 1));
	gfx_fill(s, b->r, 0xff404040u | (uint32_t)b->state);
}
