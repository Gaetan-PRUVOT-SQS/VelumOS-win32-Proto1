use crate::{
    SET_LAYOUT_AZERTY, SET_MODES, SET_NIGHT_FROM, SET_NIGHT_TO, SET_THEME_DARK, SET_THEME_LIGHT,
    t_setmode, t_settings,
};

const WH: [t_setmode; SET_MODES as usize] = [
    t_setmode { w: 1024, h: 768 },
    t_setmode { w: 1280, h: 720 },
    t_setmode { w: 1366, h: 768 },
    t_setmode { w: 1600, h: 900 },
    t_setmode { w: 1920, h: 1080 },
];

pub(crate) fn apply_default(s: &mut t_settings) {
    s.theme = SET_THEME_LIGHT;
    s.reduced = 0;
    s.layout = SET_LAYOUT_AZERTY;
    s.mode = 1;
}

pub(crate) fn mode_at(index: u32) -> t_setmode {
    let i = if index >= SET_MODES { 0 } else { index };
    WH[i as usize]
}

pub(crate) fn dark_at(s: &t_settings, hour: u32) -> bool {
    if s.theme == SET_THEME_DARK {
        return true;
    }
    if s.theme == SET_THEME_LIGHT {
        return false;
    }
    !(SET_NIGHT_TO..SET_NIGHT_FROM).contains(&hour)
}
