#include "display_int.h"
#include "kfix.h"

static bool	match_at(const char *ascii, uint32_t from, uint32_t total)
{
	uint32_t	i;

	i = 0;
	while (ascii[i])
	{
		if (from + i >= total)
			return (false);
		if (g_ffont.log[(from + i) % FAKE_LOG].cp != (uint8_t)ascii[i])
			return (false);
		i++;
	}
	return (true);
}

bool	fake_log_has(const char *ascii)
{
	uint32_t	first;
	uint32_t	from;

	first = 0;
	if (g_ffont.glyphs > FAKE_LOG)
		first = g_ffont.glyphs - FAKE_LOG;
	from = first;
	while (from < g_ffont.glyphs)
	{
		if (match_at(ascii, from, g_ffont.glyphs))
			return (true);
		from++;
	}
	return (false);
}

bool	fx_display(uint32_t w, uint32_t h)
{
	fake_reset();
	fake_fb_set(w, h);
	fake_cmdline("");
	display_boot_init();
	return (display_ready());
}
