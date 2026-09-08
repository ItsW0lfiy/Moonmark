# Qt licensing notes

Moonmark currently builds against Qt 6.11.2 and uses only Qt Core, Gui, and Widgets plus the `qwindows` platform plugin. Those essential modules are available under Qt's commercial license or the open-source LGPLv3/GPLv3 options; none of Moonmark's used modules appears in Qt's GPL-only module list.

The measured portable layout dynamically links Qt under the intended open-source evaluation path. Dynamic linking can allow Moonmark's own source to use separate terms when all LGPLv3 conditions are satisfied, including prominent notice, license copies, user replacement/relinking rights, installation information where applicable, and provision of the complete corresponding Qt source (including modifications) or a compliant written offer under the distributor's control.

Static linking is not equivalent. Qt's own guidance recommends dynamic linking or making application source available under LGPL; LGPLv3's combined-work requirements can require relinkable application material and installation information. Moonmark has no approved distribution license yet, so this migration does not publish a static binary or choose GPL/commercial terms.

`docs/licenses/Qt-LGPL-3.0-only.txt` and `docs/licenses/Qt-GPL-3.0-only.txt` are verbatim license texts from the Qt 6.11.2 `qtbase` source tree. `THIRD_PARTY_NOTICES.txt` records the current notice and publication blocker. Qt's bundled third-party components have their own licenses; the Qt 6.11.2 SBOM should be used to generate the exact release notices for the final shipped binaries.

This document is an engineering audit, not legal advice. Public binary distribution remains blocked on choosing Moonmark's own license and completing a release-specific compliance review/source-offer process.

