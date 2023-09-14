Arm GNU Toolchain 11.3.rel1
GNU toolchain for the Arm architecture
GCC Version: 11.3

For updated content, see the release note for the relevant release, on:
https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/downloads

****************************************************
Infineon modifications
****************************************************

Below is a list of modifications applied by Infineon
to the Arm GNU Toolchain prebuilt package:

* Stripped debug symbols from the target libraries
  (libc_nano.a, libm.a, ...) to reduce toolchain size.

* Removed '.gnu.warning.*' sections from libnosys.a
  libraries (to suppress irrelevant linker warning).

* Corrected newlib version macros in _newlib_version.h
  to address https://bugs.linaro.org/show_bug.cgi?id=5894

* Recompiled Linux GDB binary (arm-none-eabi-gdb)
  without Python support (--with-python=no),
  to address https://bugs.linaro.org/show_bug.cgi?id=5842

