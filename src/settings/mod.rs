#[derive(Clone, Debug)]
pub struct Settings {
    pub body_font_points: f32,
    pub document_padding: i32,
    pub line_height_percent: i32,
    pub image_cache_bytes: usize,
    pub image_workers: usize,
}

impl Default for Settings {
    fn default() -> Self {
        Self {
            body_font_points: 12.75,
            document_padding: 48,
            line_height_percent: 150,
            image_cache_bytes: 128 * 1024 * 1024,
            image_workers: 4,
        }
    }
}
