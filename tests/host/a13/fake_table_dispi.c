#include "cases.h"
#include "velum/err.h"

const t_modecase	g_mode_cases[] = {
{"800x600", 800, 600, true},
{"1024x768", 1024, 768, true},
{"1280x720", 1280, 720, true},
{"1280x1024", 1280, 1024, true},
{"1920x1080", 1920, 1080, true},
{"640x480 hors liste", 640, 480, false},
{"1600x900 hors liste", 1600, 900, false},
{"1920x1200 hors liste", 1920, 1200, false},
{"0x0", 0, 0, false},
{"800x0", 800, 0, false},
{"0x600", 0, 600, false},
{"transpose 600x800", 600, 800, false},
{"65535x65535", 65535, 65535, false},
{"max u32", UINT32_MAX, UINT32_MAX, false},
{"1280x721", 1280, 721, false},
{"801x600", 801, 600, false},
{NULL, 0, 0, false}
};

const t_idcase		g_id_cases[] = {
{"id B0C0 sans LFB", 45248, 256, 16777216, E_NODEV, 0},
{"id B0C1 sans LFB", 45249, 256, 16777216, E_NODEV, 0},
{"id B0C2 premier avec LFB", 45250, 256, 16777216, E_OK, 16777216},
{"id B0C5", 45253, 256, 16777216, E_OK, 16777216},
{"id B0CF derniere acceptee", 45263, 256, 16777216, E_OK, 16777216},
{"id B0D0 refusee", 45264, 256, 16777216, E_NODEV, 0},
{"id FFFF appareil absent", 65535, 256, 16777216, E_NODEV, 0},
{"vram 0 bar 8 Mio", 45253, 0, 8388608, E_OK, 8388608},
{"vram 16 Mio bar 8 Mio", 45253, 256, 8388608, E_OK, 8388608},
{"vram 8 Mio bar 16 Mio", 45253, 128, 16777216, E_OK, 8388608},
{"vram 16 Mio bar 0", 45253, 256, 0, E_OK, 16777216},
{"vram 0 bar 0", 45253, 0, 0, E_NODEV, 0},
{"vram 65535 unites bar 16 Mio", 45253, 65535, 16777216, E_OK, 16777216},
{NULL, 0, 0, 0, 0, 0}
};
