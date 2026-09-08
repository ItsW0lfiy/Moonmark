pub mod convert;
mod highlight;
pub mod model;
pub mod parser;

pub use convert::to_presentation;
pub use model::{Block, DocumentModel, Inline};
pub use parser::parse;
