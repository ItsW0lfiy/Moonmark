use std::collections::HashMap;
use std::path::Path;
use std::time::Instant;

use crate::images::{ImagePipeline, ImageRequest, ImageResult};
use crate::markdown::{parse, to_presentation};
use crate::presentation::{PresentationDocument, PresentationMetrics};
use crate::settings::Settings;

pub struct Backend {
    settings: Settings,
    revision: u64,
    parse_count: u64,
    load_count: u64,
    image_pipeline: ImagePipeline,
    image_requests: HashMap<u32, ImageRequest>,
}

impl Default for Backend {
    fn default() -> Self {
        let settings = Settings::default();
        let image_pipeline = ImagePipeline::new(settings.image_workers, settings.image_cache_bytes);
        Self {
            settings,
            revision: 0,
            parse_count: 0,
            load_count: 0,
            image_pipeline,
            image_requests: HashMap::new(),
        }
    }
}

impl Backend {
    pub fn open_document(&mut self, path: &str) -> PresentationDocument {
        let path = Path::new(path);
        let read_started = Instant::now();
        let source = match std::fs::read_to_string(path) {
            Ok(source) => source,
            Err(error) => {
                return PresentationDocument::error(format!("Could not read document: {error}"));
            }
        };
        let read_us = elapsed_us(read_started);
        let canonical = match std::fs::canonicalize(path) {
            Ok(path) => path,
            Err(error) => {
                return PresentationDocument::error(format!(
                    "Could not resolve document path: {error}"
                ));
            }
        };
        self.load_count = self.load_count.saturating_add(1);
        self.revision = self.revision.saturating_add(1);
        self.image_pipeline.reset_requests();

        let parse_started = Instant::now();
        let model = parse(&source);
        let parse_us = elapsed_us(parse_started);
        self.parse_count = self.parse_count.saturating_add(1);
        let (mut presentation, requests) = to_presentation(
            &model,
            &canonical,
            PresentationMetrics {
                revision: self.revision,
                source_bytes: source.len() as u64,
                read_us,
                parse_us,
                ..PresentationMetrics::default()
            },
            &self.settings,
        );
        self.image_requests = requests
            .into_iter()
            .map(|request| (request.id, request))
            .collect();
        presentation.metrics.parse_count = self.parse_count;
        presentation.metrics.load_count = self.load_count;
        presentation
    }

    pub fn queue_image(&mut self, id: u32, max_width: u32) -> bool {
        let Some(request) = self.image_requests.get(&id) else {
            return false;
        };
        let mut request = request.clone();
        request.max_width = max_width;
        self.image_pipeline.queue(request);
        true
    }

    pub fn poll_images(&self) -> Vec<ImageResult> {
        self.image_pipeline.poll()
    }

    pub fn poll_image(&self) -> Option<ImageResult> {
        self.image_pipeline.poll_one()
    }

    pub fn image_requests(&self) -> Vec<ImageRequest> {
        self.image_requests.values().cloned().collect()
    }

    pub fn cache_cost(&self) -> u64 {
        self.image_pipeline.cache_cost()
    }

    pub fn parse_count(&self) -> u64 {
        self.parse_count
    }
    pub fn load_count(&self) -> u64 {
        self.load_count
    }
    pub fn image_request_count(&self) -> u64 {
        self.image_pipeline.request_count()
    }
}

fn elapsed_us(started: Instant) -> u64 {
    u64::try_from(started.elapsed().as_micros()).unwrap_or(u64::MAX)
}
