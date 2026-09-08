use std::collections::HashMap;

#[derive(Clone)]
pub struct CachedImage {
    pub width: u32,
    pub height: u32,
    pub rgba: Vec<u8>,
}

struct Entry {
    image: CachedImage,
    touched: u64,
}

pub struct ImageCache {
    entries: HashMap<String, Entry>,
    budget: usize,
    cost: usize,
    clock: u64,
}

impl ImageCache {
    pub fn new(budget: usize) -> Self {
        Self {
            entries: HashMap::new(),
            budget,
            cost: 0,
            clock: 0,
        }
    }

    pub fn get(&mut self, key: &str) -> Option<CachedImage> {
        self.clock = self.clock.wrapping_add(1);
        let entry = self.entries.get_mut(key)?;
        entry.touched = self.clock;
        Some(entry.image.clone())
    }

    pub fn insert(&mut self, key: String, image: CachedImage) {
        let image_cost = image.rgba.len();
        if image_cost > self.budget {
            return;
        }
        if let Some(previous) = self.entries.remove(&key) {
            self.cost = self.cost.saturating_sub(previous.image.rgba.len());
        }
        self.clock = self.clock.wrapping_add(1);
        self.cost += image_cost;
        self.entries.insert(
            key,
            Entry {
                image,
                touched: self.clock,
            },
        );
        while self.cost > self.budget {
            let Some(oldest) = self
                .entries
                .iter()
                .min_by_key(|(_, entry)| entry.touched)
                .map(|(key, _)| key.clone())
            else {
                break;
            };
            if let Some(entry) = self.entries.remove(&oldest) {
                self.cost = self.cost.saturating_sub(entry.image.rgba.len());
            }
        }
    }

    pub fn cost(&self) -> usize {
        self.cost
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn evicts_to_its_explicit_budget() {
        let mut cache = ImageCache::new(8);
        cache.insert(
            "a".into(),
            CachedImage {
                width: 1,
                height: 1,
                rgba: vec![0; 8],
            },
        );
        cache.insert(
            "b".into(),
            CachedImage {
                width: 1,
                height: 1,
                rgba: vec![0; 8],
            },
        );
        assert!(cache.cost() <= 8);
        assert!(cache.get("a").is_none());
    }
}
