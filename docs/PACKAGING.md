# Windows packaging

`scripts/package_windows.ps1` builds Release and assembles an ignored `deploy/Moonmark` folder plus `deploy/Moonmark-portable-win-x64.zip`.

The portable folder contains:

- `Moonmark.exe`
- dynamically linked Qt6Core, Qt6Gui, and Qt6Widgets
- the Windows QPA plugin at `platforms/qwindows.dll`
- app-local MSVC runtime DLLs when the Visual Studio redistributable directory is available
- approved symbol artwork (the application, shell, and EXE icon are embedded and do not depend on this file)
- `qt.conf`
- Qt LGPL/GPL texts and Moonmark's third-party notice

This ZIP is the current practical portable architecture. It needs no .NET, JVM, Node.js, browser engine, or separately installed Qt. It uses normal Windows system libraries; including app-local MSVC CRT DLLs avoids asking users to install the Visual C++ Redistributable separately.

The Qt SDK, Rust toolchain, Cargo, MSVC compiler, and Windows SDK are build-time requirements only. Normal users receive the executable, exact Qt DLLs/plugins, app-local CRT where needed, assets, notices, and license texts. The portable edition runs directly from its extracted directory; the future installer will install essentially the same payload rather than bootstrap a development SDK or language runtime.

An installer should deploy the same files, then add approved shortcuts, uninstall metadata, and file associations. Installer technology has not been selected, so no installer is built in this milestone.

A true single executable requires a separate static Qt build and a deliberate Qt licensing decision. It was not built. Under LGPLv3, static distribution adds relinking/application-object and installation-information obligations and may affect whether the application remains merely a work using the library. Dynamic Qt is the safer current packaging choice; legal review and Moonmark's own license decision remain required before public distribution.

Dev.6 measured 6,388,736 bytes for Moonmark.exe, 35,021,676 bytes (33.40 MiB) for the complete 16-file folder, and 15,530,358 bytes (14.81 MiB) for the ZIP. The remaining 28,632,940 bytes are Qt, the Windows platform plugin, app-local MSVC CRT, branding, and legal/support files. EXE/DLL import inspection found Qt, app-local MSVC CRT, and Windows system dependencies, including system ICU. Packaged icon/render/style/multi-document/layout/image smokes passed with PATH limited to Windows directories and Qt SDK discovery variables cleared. This is not a clean-VM or minimum-Windows-version certification; see [dev.6 validation](DEV6_VALIDATION.md).
