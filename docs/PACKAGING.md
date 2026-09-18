# Windows packaging

`scripts/package_windows.ps1` is the release-grade Windows packaging entry point:

```powershell
pwsh -File scripts/package_windows.ps1
```

It reads the version from Cargo metadata, builds Release, stages an ignored `deploy/staging/<version>/Moonmark` folder, smoke-tests that app-local payload with Qt discovery variables cleared and `PATH` limited to Windows system directories, then writes the release artifacts to `deploy/release/<version>/`.

The current engine-independent artifacts are:

- `Moonmark-portable-win-x64.zip`
- `SHA256SUMS.txt`

After an installer engine is explicitly approved, the same script can accept its completed artifact through `-InstallerPath`; it stages that file as `Moonmark-Setup-win-x64.exe` and regenerates checksums over both artifacts. The script does not build or assume an installer engine itself.

The portable folder contains:

- `Moonmark.exe`
- dynamically linked Qt6Core, Qt6Gui, and Qt6Widgets
- the Windows QPA plugin at `platforms/qwindows.dll`
- app-local MSVC runtime DLLs when the Visual Studio redistributable directory is available
- approved symbol artwork (the application, shell, and EXE icon are embedded and do not depend on this file)
- `qt.conf`
- Qt LGPL/GPL texts and Moonmark's third-party notice

This ZIP is the current practical portable architecture. It needs no .NET, JVM, Node.js, browser engine, or separately installed Qt. It uses normal Windows system libraries; including app-local MSVC CRT DLLs avoids asking users to install the Visual C++ Redistributable separately.

The Qt SDK, Rust toolchain, Cargo, MSVC compiler, Windows SDK, and `vswhere.exe` are build-time requirements only. `vswhere` locates installed C++ toolchains and the newest app-local x64 VC runtime instead of relying on a hardcoded Visual Studio edition path. Missing Qt, platform-plugin, or CRT files fail packaging instead of silently producing a client-dependent archive. Normal users receive the executable, exact Qt DLLs/plugins, app-local CRT, assets, notices, and license texts. The portable edition runs directly from its extracted directory; the future installer will install essentially the same payload rather than bootstrap a development SDK or language runtime.

An installer should deploy the same staged payload, then add approved shortcuts, uninstall metadata, and file associations. Installer technology still requires explicit approval, so the current dev.7 bundle intentionally has no setup executable yet.

A true single executable requires a separate static Qt build and a deliberate Qt licensing decision. It was not built. Under LGPLv3, static distribution adds relinking/application-object and installation-information obligations and may affect whether the application remains merely a work using the library. Dynamic Qt is the safer current packaging choice; legal review and Moonmark's own license decision remain required before public distribution.

The first dev.7 package measurement is 6,496,768 bytes for `Moonmark.exe`, 35,264,372 bytes (33.63 MiB) for the complete portable folder, and 15,625,972 bytes (14.90 MiB) for the ZIP. The remaining folder bytes are Qt, the Windows platform plugin, app-local MSVC CRT, branding, and legal/support files. The packaged smoke passed with only normal Windows system paths visible. This is not yet a clean-VM or minimum-Windows-version certification; installer install/upgrade/uninstall validation remains gated on the installer choice.
