use std::fmt::Write as _;
use std::path::PathBuf;

fn main() -> Result<(), Box<dyn std::error::Error>> {
    let output = std::env::args_os()
        .nth(1)
        .map(PathBuf::from)
        .unwrap_or_else(|| PathBuf::from("fixtures/generated/large-literal.txt"));
    if let Some(parent) = output.parent() {
        std::fs::create_dir_all(parent)?;
    }
    let mut source = String::with_capacity(1_400_000);
    for index in 0..10_000 {
        writeln!(
            source,
            "{index:05}\t# literal heading **not bold** `not code` Unicode 月    end"
        )?;
    }
    std::fs::write(&output, source)?;
    println!("Generated {}", output.display());
    Ok(())
}
