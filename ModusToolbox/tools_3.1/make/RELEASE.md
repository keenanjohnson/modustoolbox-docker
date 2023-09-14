# Tools-make GNU make Build System Release Notes
This directory provides infrastructure for the make build system to interact with ModusToolbox.

* Supported operations:
    * Build
    * Program
    * Debug
    * IDE Integration (Eclipse, VS Code, IAR, uVision)
* Supported toolchains:
    * GCC
    * IAR
    * ARM Compiler 6

### What Changed?
#### v2.0.0
* Support ModusToolbox 3.0
* Dropped compatibility with core-make version 1.X

#### v1.6.0
* Added support for qspi-configurator to patch QSPI flashloaders. (Requires core-make-1.9.0)

#### v1.5.0
* Added support for ml configurator

#### v1.4.0
* Added support for ez-pd and lin configurator

#### v1.3.0
* Added support for opening secure-tool-configurator through ModusToolbox IDE

#### v1.2.0
* Added support for .mtb files
* Added support for tool patch releases

#### v1.1.0
* Added offline support
* Updated to use proxy-helper tool
* New gitlibs functionality for MTB 2.1 library-manager

#### v1.0.0
* Initial release

### Product/Asset Specific Instructions
Builds require that the ModusToolbox tools be installed on your machine. This comes with the ModusToolbox install. On Windows machines, it is recommended that CLI builds be executed using the Cygwin.bat located in ModusToolBox/tools\_x.y/modus-shell install directory. This guarantees a consistent shell environment for your builds.

To list the build options, run the "help" target by typing "make help" in CLI. For a verbose documentation on a specific subject type "make help CY\_HELP={variable/target}", where "variable" or "target" is one of the listed make variables or targets.

### More information
* [Infineon GitHub](https://github.com/Infineon)
* [ModusToolbox](https://www.infineon.com/cms/en/design-support/tools/sdk/modustoolbox-software)

---
© Cypress Semiconductor Corporation, 2019-2022.

