# Windows packaging

`scripts/package_windows.ps1` builds Release and assembles an ignored `deploy/Moonmark` folder plus `deploy/Moonmark-portable-win-x64.zip`.

The portable folder contains:

- `Moonmark.exe`
- dynamically linked Qt6Core, Qt6Gui, and Qt6Widgets
- the Windows QPA plugin at `platforms/qwindows.dll`
- app-local MSVC runtime DLLs when the Visual Studio redistributable directory is available
- the symbol image drawn inside the custom shell (the application/EXE icon itself is embedded)
- `qt.conf`
- Qt LGPL/GPL texts and Moonmark's third-party notice

This ZIP is the current practical portable architecture. It needs no .NET, JVM, Node.js, browser engine, or separately installed Qt. It uses normal Windows system libraries; including app-local MSVC CRT DLLs avoids asking users to install the Visual C++ Redistributable separately.

An installer should deploy the same files, then add approved shortcuts, uninstall metadata, and file associations. Installer technology has not been selected, so no installer is built in this milestone.

A true single executable requires a separate static Qt build and a deliberate Qt licensing decision. It was not built. Under LGPLv3, static distribution adds relinking/application-object and installation-information obligations and may affect whether the application remains merely a work using the library. Dynamic Qt is the safer current packaging choice; legal review and Moonmark's own license decision remain required before public distribution.
