mod cache;
mod loader;
pub mod paths;

pub use loader::ImagePipeline;
pub use paths::{AssetResolution, AssetState, resolve_local_image};

#[derive(Clone, Debug)]
pub struct ImageRequest {
    pub id: u32,
    pub path: String,
    pub max_width: u32,
}

#[derive(Clone, Debug)]
pub struct ImageResult {
    pub id: u32,
    pub width: u32,
    pub height: u32,
    pub rgba: Vec<u8>,
    pub error: String,
}
