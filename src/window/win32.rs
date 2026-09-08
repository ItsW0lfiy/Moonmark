//! Framework-neutral Windows window-state policy retained behind the native API boundary.

use super::{WindowMode, WindowState};

pub fn toggle_maximize(state: &mut WindowState, currently_maximized: bool) -> u8 {
    if state.mode == WindowMode::BorderlessFullscreen {
        return mode_code(state.mode);
    }
    state.mode = if state.mode == WindowMode::Maximized || currently_maximized {
        WindowMode::Normal
    } else {
        WindowMode::Maximized
    };
    mode_code(state.mode)
}

pub fn toggle_fullscreen(state: &mut WindowState, currently_maximized: bool) -> u8 {
    if state.mode == WindowMode::BorderlessFullscreen {
        state.leave_fullscreen_model();
    } else {
        state.mode = if currently_maximized {
            WindowMode::Maximized
        } else {
            WindowMode::Normal
        };
        state.enter_fullscreen_model();
    }
    mode_code(state.mode)
}

pub fn exit_fullscreen(state: &mut WindowState) -> u8 {
    state.leave_fullscreen_model();
    mode_code(state.mode)
}

pub const fn mode_code(mode: WindowMode) -> u8 {
    match mode {
        WindowMode::Normal => 0,
        WindowMode::Maximized => 1,
        WindowMode::BorderlessFullscreen => 2,
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn restore_uses_rust_state_after_fullscreen_even_if_frontend_flag_lags() {
        let mut state = WindowState {
            mode: WindowMode::Maximized,
            ..WindowState::default()
        };
        assert_eq!(toggle_maximize(&mut state, false), 0);
        assert_eq!(state.mode, WindowMode::Normal);
    }
}
