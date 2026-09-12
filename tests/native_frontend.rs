#![cfg(target_os = "windows")]

use std::path::PathBuf;
use std::process::Command;

fn fixture(name: &str) -> PathBuf {
    PathBuf::from(env!("CARGO_MANIFEST_DIR"))
        .join("fixtures")
        .join(name)
}

fn run_smoke(mode: &str, fixture_name: &str) -> String {
    run_smoke_many(mode, &[fixture_name])
}

fn run_smoke_many(mode: &str, fixture_names: &[&str]) -> String {
    let fixtures = fixture_names
        .iter()
        .map(|name| fixture(name))
        .collect::<Vec<_>>();
    let output = Command::new(env!("CARGO_BIN_EXE_moonmark"))
        .arg(mode)
        .args(fixtures)
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

fn numeric_metric(output: &str, name: &str) -> u64 {
    output
        .split_whitespace()
        .find_map(|field| {
            field
                .strip_prefix(&format!("{name}="))
                .and_then(|value| value.parse().ok())
        })
        .unwrap_or_else(|| panic!("missing numeric metric {name} in {output}"))
}

#[test]
fn outline_navigation_survives_image_reflow_with_one_click() {
    for fixture_name in [
        "outline-reflow-single.md",
        "outline-reflow-multiple.md",
        "outline-reflow-missing.md",
        "outline-reflow-failed.md",
        "outline-reflow-bottom.md",
        "outline-reflow-rapid.md",
    ] {
        let output = run_smoke("--smoke-outline-reflow", fixture_name);
        assert!(
            output.contains("outline_reflow=ok"),
            "{fixture_name}: {output}"
        );
        assert!(output.contains("one_click=ok"), "{fixture_name}: {output}");
        assert!(
            output.contains("latest_wins=ok"),
            "{fixture_name}: {output}"
        );
        assert!(
            output.contains("counters=stable"),
            "{fixture_name}: {output}"
        );
        assert!(
            output.contains("motion=stopped"),
            "{fixture_name}: {output}"
        );
    }
}

#[test]
fn qt_frontend_smoke_matrix() {
    let output = run_smoke("--smoke-image-geometry", "image-layout-regression.md");
    assert!(output.contains("images=ok"), "{output}");
    let output = run_smoke("--smoke-render", "inline-code-quality.md");
    assert!(output.contains("selection_copy=ok"), "{output}");
    let output = run_smoke("--smoke-navigation", "concept-presentation.md");
    assert!(output.contains("navigation=ok"), "{output}");
    let output = run_smoke("--smoke-motion", "navigation-motion.md");
    assert!(output.contains("motion=ok"), "{output}");
    assert!(output.contains("counters=stable"), "{output}");
    assert!(output.contains("partial_wheel=ok"), "{output}");
    assert!(output.contains("cancelled=ok"), "{output}");
    assert!(
        output.contains("pixel=direct home=direct end=direct"),
        "{output}"
    );

    let output = run_smoke("--smoke-scroll-profile", "generated/large-text.md");
    assert!(output.contains("scroll_profile=ok"), "{output}");
    assert!(output.contains("paint_interval_ms_p50="), "{output}");
    assert!(output.contains("over_50="), "{output}");
    let controller_frames = numeric_metric(&output, "controller_frames");
    let image_scans = numeric_metric(&output, "image_scans");
    assert!(controller_frames > 20, "{output}");
    assert!(image_scans * 2 < controller_frames, "{output}");

    let output = run_smoke("--smoke-plaintext", "text/literal.txt");
    assert!(output.contains("plaintext=ok"), "{output}");
    assert!(output.contains("parse_count=0"), "{output}");

    let output = run_smoke_many(
        "--smoke-multidoc",
        &["code-block-quality.md", "text/literal.txt"],
    );
    assert!(output.contains("multidoc=ok"), "{output}");
    assert!(output.contains("duplicate=deduplicated"), "{output}");
    assert!(output.contains("state=retained"), "{output}");
    assert!(output.contains("counters=stable"), "{output}");
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

#[test]
fn qt_background_text_watcher_updates_only_its_session() {
    let watcher_root = PathBuf::from(env!("CARGO_MANIFEST_DIR"))
        .join("target")
        .join("moonmark-native-tests");
    std::fs::create_dir_all(&watcher_root).expect("create native test directory");
    let active_fixture = watcher_root.join("active.md");
    let background_fixture = watcher_root.join("background.txt");
    std::fs::write(
        &active_fixture,
        "# Active fixture\n\nDo not reload this document.\n",
    )
    .expect("write active watcher fixture");
    std::fs::write(&background_fixture, "Literal **background** text.\n")
        .expect("write background watcher fixture");
    let output = Command::new(env!("CARGO_BIN_EXE_moonmark"))
        .arg("--smoke-multidoc-watcher")
        .arg(&active_fixture)
        .arg(&background_fixture)
        .current_dir(env!("CARGO_MANIFEST_DIR"))
        .output()
        .expect("launch Moonmark background watcher smoke test");
    let stdout = String::from_utf8_lossy(&output.stdout);
    let stderr = String::from_utf8_lossy(&output.stderr);
    assert!(
        output.status.success(),
        "Moonmark background watcher smoke failed\nstdout:\n{stdout}\nstderr:\n{stderr}"
    );
    assert!(stdout.contains("multidoc_watcher=ok"), "{stdout}");
    assert!(stdout.contains("active=stable"), "{stdout}");
    assert!(stdout.contains("background_parse_delta=0"), "{stdout}");
    std::fs::remove_file(active_fixture).expect("remove active watcher fixture");
    std::fs::remove_file(background_fixture).expect("remove background watcher fixture");
}
