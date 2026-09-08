#[derive(Clone, Copy, Debug, Eq, PartialEq)]
pub enum WindowMode {
    Normal,
    Maximized,
    BorderlessFullscreen,
}

#[derive(Debug)]
pub struct WindowState {
    pub mode: WindowMode,
    pub pre_fullscreen_mode: WindowMode,
    pub saved_style: isize,
    pub placement: [i32; 11],
}

impl Default for WindowState {
    fn default() -> Self {
        Self {
            mode: WindowMode::Normal,
            pre_fullscreen_mode: WindowMode::Normal,
            saved_style: 0,
            placement: [0; 11],
        }
    }
}

impl WindowState {
    pub fn save_normal_geometry(&mut self, x: i32, y: i32, width: i32, height: i32) {
        self.placement[..5].copy_from_slice(&[x, y, width, height, 1]);
    }

    pub fn normal_geometry(&self) -> Option<[i32; 4]> {
        (self.placement[4] == 1).then_some([
            self.placement[0],
            self.placement[1],
            self.placement[2],
            self.placement[3],
        ])
    }

    pub fn enter_fullscreen_model(&mut self) {
        if self.mode != WindowMode::BorderlessFullscreen {
            self.pre_fullscreen_mode = self.mode;
            self.mode = WindowMode::BorderlessFullscreen;
        }
    }

    pub fn leave_fullscreen_model(&mut self) -> bool {
        if self.mode != WindowMode::BorderlessFullscreen {
            return false;
        }
        self.mode = self.pre_fullscreen_mode;
        true
    }

    pub fn toggle_maximize_model(&mut self) {
        if self.mode == WindowMode::BorderlessFullscreen {
            return;
        }
        self.mode = if self.mode == WindowMode::Maximized {
            WindowMode::Normal
        } else {
            WindowMode::Maximized
        };
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn fullscreen_restores_normal() {
        let mut state = WindowState::default();
        state.enter_fullscreen_model();
        assert!(state.leave_fullscreen_model());
        assert_eq!(state.mode, WindowMode::Normal);
    }

    #[test]
    fn fullscreen_restores_maximized() {
        let mut state = WindowState::default();
        state.toggle_maximize_model();
        state.enter_fullscreen_model();
        assert!(state.leave_fullscreen_model());
        assert_eq!(state.mode, WindowMode::Maximized);
    }

    #[test]
    fn escape_is_inert_outside_fullscreen() {
        let mut state = WindowState::default();
        assert!(!state.leave_fullscreen_model());
        assert_eq!(state.mode, WindowMode::Normal);
    }

    #[test]
    fn repeated_fullscreen_toggle_preserves_meaning() {
        let mut state = WindowState::default();
        state.enter_fullscreen_model();
        state.leave_fullscreen_model();
        state.enter_fullscreen_model();
        state.leave_fullscreen_model();
        assert_eq!(state.mode, WindowMode::Normal);
    }

    #[test]
    fn normal_geometry_round_trips_through_rust_state() {
        let mut state = WindowState::default();
        state.save_normal_geometry(120, 80, 1280, 760);
        assert_eq!(state.normal_geometry(), Some([120, 80, 1280, 760]));
    }
}
