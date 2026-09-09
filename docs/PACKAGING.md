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

An installer should deploy the same files, then add approved shortcuts, uninstall metadata, and file associations. Installer technology has not been selected, so no installer is built in this milestone.

A true single executable requires a separate static Qt build and a deliberate Qt licensing decision. It was not built. Under LGPLv3, static distribution adds relinking/application-object and installation-information obligations and may affect whether the application remains merely a work using the library. Dynamic Qt is the safer current packaging choice; legal review and Moonmark's own license decision remain required before public distribution.

Dev.4 measured 6,339,072 bytes for Moonmark.exe, 34,936,739 bytes (33.32 MiB) for the complete folder, and 15,494,608 bytes (14.78 MiB) for the ZIP. EXE/DLL import inspection found Qt, app-local MSVC CRT, and Windows system dependencies, including system ICU. Packaged icon/render/style/navigation smoke tests passed with PATH limited to Windows directories and Qt SDK discovery variables cleared. This is not a clean-VM or minimum-Windows-version certification; see [dev.4 validation](DEV4_VALIDATION.md).
