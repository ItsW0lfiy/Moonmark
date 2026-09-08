#[derive(Clone, Copy, Debug, Eq, PartialEq)]
pub struct Rgb(pub u8, pub u8, pub u8);

impl Rgb {
    #[must_use]
    pub const fn is_achromatic(self) -> bool {
        self.0 == self.1 && self.1 == self.2
    }
}

pub const BACKGROUND: Rgb = Rgb(8, 8, 8);
pub const SHELL: Rgb = Rgb(12, 12, 12);
pub const DOCUMENT: Rgb = Rgb(14, 14, 14);
pub const SURFACE: Rgb = Rgb(20, 20, 20);
pub const RAISED: Rgb = Rgb(28, 28, 28);
pub const HOVER: Rgb = Rgb(36, 36, 36);
pub const ACTIVE: Rgb = Rgb(48, 48, 48);
pub const BORDER: Rgb = Rgb(48, 48, 48);
pub const BORDER_STRONG: Rgb = Rgb(70, 70, 70);
pub const TEXT: Rgb = Rgb(232, 232, 232);
pub const TEXT_SECONDARY: Rgb = Rgb(176, 176, 176);
pub const TEXT_MUTED: Rgb = Rgb(124, 124, 124);
pub const TEXT_DISABLED: Rgb = Rgb(82, 82, 82);
pub const SILVER: Rgb = Rgb(200, 200, 200);
pub const BRIGHT_SILVER: Rgb = Rgb(240, 240, 240);
pub const FOCUS: Rgb = Rgb(216, 216, 216);
pub const SELECTION: Rgb = Rgb(72, 72, 72);
pub const ERROR: Rgb = Rgb(190, 104, 104);

pub const ALL_DEFAULT_COLOURS: &[Rgb] = &[
    BACKGROUND,
    SHELL,
    DOCUMENT,
    SURFACE,
    RAISED,
    HOVER,
    ACTIVE,
    BORDER,
    BORDER_STRONG,
    TEXT,
    TEXT_SECONDARY,
    TEXT_MUTED,
    TEXT_DISABLED,
    SILVER,
    BRIGHT_SILVER,
    FOCUS,
    SELECTION,
];

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn default_theme_is_achromatic() {
        assert!(ALL_DEFAULT_COLOURS.iter().copied().all(Rgb::is_achromatic));
    }
}
