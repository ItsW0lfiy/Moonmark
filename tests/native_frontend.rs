#![cfg(target_os = "windows")]

use std::path::PathBuf;
use std::process::Command;

fn fixture(name: &str) -> PathBuf {
    PathBuf::from(env!("CARGO_MANIFEST_DIR"))
        .join("fixtures")
        .join(name)
}

fn run_smoke(mode: &str, fixture_name: &str) -> String {
    let output = Command::new(env!("CARGO_BIN_EXE_moonmark"))
        .arg(mode)
        .arg(fixture(fixture_name))
        .current_dir(env!("CARGO_MANIFEST_DIR"))
        .output()
        .expect("launch Moonmark native smoke test");
    assert!(
        output.status.success(),
        "Moonmark smoke failed ({:?}): {}{}",
        output.status.code(),
        String::from_utf8_lossy(&output.stdout),
        String::from_utf8_lossy(&output.stderr)
    );
    String::from_utf8_lossy(&output.stdout).into_owned()
}

#[test]
fn qt_frontend_smoke_matrix() {
    let output = run_smoke("--smoke-image-geometry", "image-layout-regression.md");
    assert!(output.contains("images=ok"), "{output}");
    let output = run_smoke("--smoke-navigation", "concept-presentation.md");
    assert!(output.contains("navigation=ok"), "{output}");
    let output = run_smoke("--smoke-zoom", "moonmark-visual-test.md");
    assert!(output.contains("zoom=ok"), "{output}");
    assert!(output.contains("image_request_delta=0"), "{output}");

    let output = run_smoke("--smoke-render", "code-block-quality.md");
    assert!(output.contains("render=ok"), "{output}");
    assert!(output.contains("selection_copy=ok"), "{output}");
    assert!(output.contains("code_copy=ok"), "{output}");

    let output = run_smoke("--smoke-style", "document-tables.md");
    assert!(output.contains("document_style=ok"), "{output}");

    let output = run_smoke("--smoke-layout", "layout-transitions.md");
    assert!(output.contains("layout=ok"), "{output}");
    assert!(output.contains("parse_delta=0"), "{output}");
    assert!(output.contains("load_delta=0"), "{output}");
    assert!(output.contains("construction_delta=0"), "{output}");
    assert!(output.contains("image_request_delta=0"), "{output}");

    let output = run_smoke("--smoke-maximize", "layout-transitions.md");
    assert!(output.contains("maximize=ok"), "{output}");
    assert!(output.contains("geometry=restored"), "{output}");

    let output = run_smoke("--smoke-layout-normal", "layout-transitions.md");
    assert!(output.contains("layout_normal=ok"), "{output}");
    assert!(output.contains("geometry=restored"), "{output}");
    let output = run_smoke("--smoke-images", "moonmark-visual-test.md");
    assert!(output.contains("images=ok"), "{output}");
    assert!(output.contains("failed=0"), "{output}");
    assert!(output.contains("pending=0"), "{output}");

    let watcher_root = PathBuf::from(env!("CARGO_MANIFEST_DIR"))
        .join("target")
        .join("moonmark-native-tests");
    std::fs::create_dir_all(&watcher_root).expect("create native test directory");
    let watcher_fixture = watcher_root.join("watcher.md");
    std::fs::write(&watcher_fixture, "# Watcher fixture\n\nOriginal content.\n")
        .expect("write watcher fixture");
    let output = Command::new(env!("CARGO_BIN_EXE_moonmark"))
        .arg("--smoke-watcher")
        .arg(&watcher_fixture)
        .current_dir(env!("CARGO_MANIFEST_DIR"))
        .output()
        .expect("launch Moonmark watcher smoke test");
    assert!(
        output.status.success(),
        "{}{}",
        String::from_utf8_lossy(&output.stdout),
        String::from_utf8_lossy(&output.stderr)
    );
    let stdout = String::from_utf8_lossy(&output.stdout);
    assert!(stdout.contains("watcher=ok"), "{stdout}");
    std::fs::remove_file(watcher_fixture).expect("remove watcher fixture");
}
