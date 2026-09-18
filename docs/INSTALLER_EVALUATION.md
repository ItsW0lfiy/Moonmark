# Windows installer evaluation — dev.7

Moonmark does not yet have an approved installer engine. This evaluation records the dev.7 decision gate; it does not authorize or add an installer dependency.

## Shortlist

| Engine | Output and unattended use | Upgrade, uninstall, shell integration | Build/tooling and license | Moonmark fit |
| --- | --- | --- | --- | --- |
| **Inno Setup** | Traditional single `.exe`; documented `/SILENT` and `/VERYSILENT` modes with `/SUPPRESSMSGBOXES` and `/NORESTART` | Built-in uninstall, shortcuts, Program Files installs, registry entries, version-aware replacement, and optional installer tasks | Native compiler plus an `.iss` installer DSL. Its license permits use for any purpose, including commercial applications; the project separately requests commercial users purchase a license. No client runtime is installed. | **Recommended.** Smallest maintainable route to Moonmark's conventional installer and future WinGet validation. |
| **NSIS** | Traditional `.exe`; `/S` silent mode | Fully capable through its script language, but upgrades, registry ownership, and uninstall details are more manual | Native compiler and `.nsi` DSL; primarily zlib/libpng licensed, with separately licensed compression modules | Viable runner-up, but Moonmark would own more low-level installation logic for no current benefit. |
| **WiX Toolset** | Native `.msi`/bundle output and strong enterprise unattended behavior | Excellent Windows Installer upgrade/uninstall semantics and component ownership | Current WiX is normally built as a .NET tool/MSBuild SDK and requires a .NET SDK. Current documentation also describes an Open Source Maintenance Fee for revenue-generating use. | Not recommended under Moonmark's current runtime/no-required-paid-component policy without another explicit decision. |
| **MSIX** | Native `.msix`; deployment is largely Windows-managed | Strong clean install/update/uninstall and declarative file associations | Windows tooling; every directly deployed package must be signed and the certificate trusted on the client | Poor fit for an unsigned first GitHub prerelease. It also introduces package identity/container and signing decisions beyond this milestone. |

## Recommendation and approval gate

Use **Inno Setup** for the first Moonmark installer, subject to explicit user approval. It produces the expected offline setup EXE, can install the already validated portable payload under Program Files, can register Moonmark without taking defaults, supports optional Desktop/file-association tasks, has normal uninstall/upgrade behavior, and has unattended switches suitable for later WinGet validation.

Approval would authorize adding an `.iss` source file and invoking the Inno compiler from the existing PowerShell release workflow. It would not authorize dev.8 update logic, code signing claims, single-instance IPC, or changing the application architecture.

Until approval, dev.7 can produce and validate only:

- `Moonmark-portable-win-x64.zip`
- `SHA256SUMS.txt`
- the exact installer input/staging directory
- installer-independent Windows executable metadata and shell argument handling

Install/upgrade/uninstall and real Explorer Open With registration remain blocked on this one decision.

## Sources checked

- [Inno Setup features](https://jrsoftware.org/isinfo.php), [license](https://github.com/jrsoftware/issrc/blob/main/license.txt), and [command-line parameters](https://jrsoftware.org/ishelp/topic_setupcmdline.htm)
- [NSIS license](https://nsis.sourceforge.io/License) and [silent install behavior](https://nsis.sourceforge.io/Docs/Chapter3.html#3.2.1)
- [WiX usage and .NET SDK requirement](https://docs.firegiant.com/wix/using-wix/) and [current licensing/maintenance-fee statement](https://docs.firegiant.com/wix/)
- [Microsoft's Windows packaging comparison](https://learn.microsoft.com/windows/apps/package-and-deploy/packaging/) and [MSIX signing requirement](https://learn.microsoft.com/windows/msix/package/signing-package-overview)

Licensing findings are an engineering audit, not legal advice.
