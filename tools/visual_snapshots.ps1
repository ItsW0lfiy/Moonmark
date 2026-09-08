param([string]$Executable = 'target/debug/moonmark.exe')
$ErrorActionPreference = 'Stop'
$cases = @(
    @('empty', '', 1200, 820, 0),
    @('prose', 'fixtures/moonmark-visual-test.md', 1200, 820, 0),
    @('headings-lists', 'fixtures/moonmark-visual-test.md', 1200, 820, 300),
    @('tables', 'fixtures/document-tables.md', 1200, 960, 0),
    @('code', 'fixtures/code-block-quality.md', 1200, 900, 0),
    @('narrow', 'fixtures/document-tables.md', 720, 900, 0),
    @('wide', 'fixtures/document-tables.md', 1800, 960, 0),
    @('images', 'fixtures/generated/image-stress.md', 1200, 900, 0)
)
$names = 'MOONMARK_SNAPSHOT_NAME', 'MOONMARK_SNAPSHOT_WIDTH', 'MOONMARK_SNAPSHOT_HEIGHT', 'MOONMARK_SNAPSHOT_SCROLL'
$previous = @{}
foreach ($name in $names) { $previous[$name] = [Environment]::GetEnvironmentVariable($name, 'Process') }
try {
    foreach ($case in $cases) {
        $env:MOONMARK_SNAPSHOT_NAME = $case[0]
        $env:MOONMARK_SNAPSHOT_WIDTH = $case[2]
        $env:MOONMARK_SNAPSHOT_HEIGHT = $case[3]
        $env:MOONMARK_SNAPSHOT_SCROLL = $case[4]
        $arguments = @('--smoke-snapshot')
        if ($case[1]) {
            if (!(Test-Path -LiteralPath $case[1])) { throw "Missing fixture: $($case[1])" }
            $arguments += $case[1]
        }
        & $Executable @arguments
        if ($LASTEXITCODE -ne 0) { throw "Snapshot failed: $($case[0]) ($LASTEXITCODE)" }
    }
} finally {
    foreach ($name in $names) { [Environment]::SetEnvironmentVariable($name, $previous[$name], 'Process') }
}
