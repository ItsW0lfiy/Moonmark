[CmdletBinding()]
param()

$ErrorActionPreference = 'Stop'
$projectRoot = Split-Path -Parent $PSScriptRoot
$qtRoot = if ($env:MOONMARK_QT_DIR) { $env:MOONMARK_QT_DIR } else { Join-Path $projectRoot 'target/qt-sdk' }
$packageRoot = Join-Path $projectRoot 'deploy/Moonmark'
$zipPath = Join-Path $projectRoot 'deploy/Moonmark-portable-win-x64.zip'

Push-Location $projectRoot
try {
    & cargo build --release
    if ($LASTEXITCODE -ne 0) { throw 'Moonmark release build failed.' }

    if (Test-Path $packageRoot) { Remove-Item -Recurse -Force -LiteralPath $packageRoot }
    New-Item -ItemType Directory -Force -Path (Join-Path $packageRoot 'platforms'), (Join-Path $packageRoot 'assets/branding'), (Join-Path $packageRoot 'licenses') | Out-Null
    Copy-Item -LiteralPath 'target/release/moonmark.exe' -Destination (Join-Path $packageRoot 'Moonmark.exe')
    foreach ($name in 'Qt6Core.dll', 'Qt6Gui.dll', 'Qt6Widgets.dll') {
        Copy-Item -LiteralPath (Join-Path $qtRoot "bin/$name") -Destination $packageRoot
    }
    Copy-Item -LiteralPath (Join-Path $qtRoot 'plugins/platforms/qwindows.dll') -Destination (Join-Path $packageRoot 'platforms')
    Copy-Item -LiteralPath 'assets/branding/moonmark-symbol.png' -Destination (Join-Path $packageRoot 'assets/branding')
    Copy-Item -LiteralPath 'assets/deployment/qt.conf' -Destination (Join-Path $packageRoot 'qt.conf')
    Copy-Item -LiteralPath 'THIRD_PARTY_NOTICES.txt' -Destination $packageRoot
    Copy-Item -LiteralPath 'docs/licenses/Qt-LGPL-3.0-only.txt' -Destination (Join-Path $packageRoot 'licenses')
    Copy-Item -LiteralPath 'docs/licenses/Qt-GPL-3.0-only.txt' -Destination (Join-Path $packageRoot 'licenses')

    $redistRoots = @(
        'C:/Program Files/Microsoft Visual Studio/18/Insiders/VC/Redist/MSVC',
        'C:/Program Files (x86)/Microsoft Visual Studio/2022/BuildTools/VC/Redist/MSVC'
    )
    $redist = $redistRoots | Where-Object { Test-Path $_ } | ForEach-Object {
        Get-ChildItem -LiteralPath $_ -Directory | Sort-Object Name -Descending
    } | ForEach-Object {
        Join-Path $_.FullName 'x64/Microsoft.VC143.CRT'
    } | Where-Object { Test-Path $_ } | Select-Object -First 1
    if ($redist) {
        foreach ($name in 'msvcp140.dll', 'msvcp140_1.dll', 'msvcp140_2.dll', 'vcruntime140.dll', 'vcruntime140_1.dll') {
            $source = Join-Path $redist $name
            if (Test-Path $source) { Copy-Item -LiteralPath $source -Destination $packageRoot }
        }
    } else {
        Write-Warning 'MSVC app-local runtime files were not found; the client will need the Visual C++ Redistributable.'
    }

    $bytes = (Get-ChildItem -LiteralPath $packageRoot -File -Recurse | Measure-Object Length -Sum).Sum
    Write-Host ("Portable folder: {0:N2} MiB at {1}" -f ($bytes / 1MB), $packageRoot)
    if (Test-Path -LiteralPath $zipPath) { Remove-Item -Force -LiteralPath $zipPath }
    Compress-Archive -Path $packageRoot -DestinationPath $zipPath -CompressionLevel Optimal
    $zipBytes = (Get-Item -LiteralPath $zipPath).Length
    Write-Host ("Portable ZIP: {0:N2} MiB at {1}" -f ($zipBytes / 1MB), $zipPath)
} finally {
    Pop-Location
}
