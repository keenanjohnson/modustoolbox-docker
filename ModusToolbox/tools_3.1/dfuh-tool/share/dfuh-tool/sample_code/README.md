This instruction helps the user build a custom DFU Host tool.
The dependencies used to build an application are publicly available.

Table of Contents
- [1. Prepare build environment](#1-prepare-build-environment)
  - [1.1. Linux](#11-linux)
    - [1.1.1. Qt, CMake, Ninja](#111-qt-cmake-ninja)
    - [1.1.2. Boost](#112-boost)
    - [1.1.3. Clang (optional)](#113-clang-optional)
  - [1.2. Windows](#12-windows)
    - [1.2.1. Qt, CMake, Ninja](#121-qt-cmake-ninja)
    - [1.2.2. Boost](#122-boost)
    - [1.2.3. MSVC 2019](#123-msvc-2019)
    - [1.2.4. Bash (from Cygwin64)](#124-bash-from-cygwin64)
  - [1.3. MacOS](#13-macos)
    - [1.3.1. XCode command line tools](#131-xcode-command-line-tools)
    - [1.3.2. Qt, CMake, Ninja](#132-qt-cmake-ninja)
- [2. Build the DFU Host Tool](#2-build-the-dfu-host-tool)
  - [2.1. Update setenv.sh](#21-update-setenvsh)
  - [2.2. Run the build script](#22-run-the-build-script)
  - [2.3. Observe built executables](#23-observe-built-executables)
  - [2.4. Notes](#24-notes)
  - [2.5. Known issues](#25-known-issues)

# 1. Prepare build environment

## 1.1. Linux

### 1.1.1. Qt, CMake, Ninja
  1. Go to https://www.qt.io/download-qt-installer and select the installer for Linux.
  2. Download the installation program and execute it.
  3. Install the following components:
     - Qt 5.15.2
        - [x] Desktop gcc 64-bit
     - Developer and Designer Tools
        - [x] CMake
        - [x] Ninja
  4. You may require the following libraries: `mesa-common-dev` `libglu1-mesa-dev`. You can install them by executing the following commands
     1. `sudo apt update`
     2. `sudo apt install mesa-common-dev libglu1-mesa-dev`

### 1.1.2. Boost
  1. Download the boost 1.76.0 from https://sourceforge.net/projects/boost/files/boost/1.76.0/
  2. Unpack and build it using the steps from the Boost build guide: https://www.boost.org/doc/libs/1_76_0/more/getting_started/unix-variants.html#easy-build-and-install

### 1.1.3. Clang (optional)
  1. Install clang using the following commands
     - `sudo apt update`
     - `sudo apt install -y clang`

## 1.2. Windows

### 1.2.1. Qt, CMake, Ninja
  1. Go to https://www.qt.io/download-qt-installer and select the installer for Windows.
  2. Download the installation program and execute it.
  3. Install the following components:
     - Qt 5.15.2
        - [x] MSVC 2019 64-bit
     - Developer and Designer Tools
        - [x] CMake
        - [x] Ninja
  
### 1.2.2. Boost
  1. Download the boost 1.76.0 msvc installer from https://sourceforge.net/projects/boost/files/boost-binaries/1.76.0
  2. Install boost

### 1.2.3. MSVC 2019
  1. Download installer for Build Tools for Visual Studio 2019 Release from Microsoft website: https://visualstudio.microsoft.com/thank-you-downloading-visual-studio/?sku=BuildTools&rel=16
  2. Execute the installer.
  3. On the Workloads tab, check the `Desktop development with C++` component group.
  4. On the Individual components tab, ensure that the `MSVC v142 - VS 2019 C++ x64/x86 build tools (Latest)` is checked.
  5. Proceed with the installation.

### 1.2.4. Bash (from Cygwin64)
  1. Download Cygwin64 installer from https://www.cygwin.com/install.html
  2. Execute the installer.
  3. Make sure Root Directory is `C:\cygwin64` (default).
  4. Check the bash.
  5. Proceed with the installation
  6. Prepend `C:\cygwin64\bin` to PATH.

## 1.3. MacOS

### 1.3.1. XCode command line tools
  1. Install the XCode command line tools with the command `sudo xcode-select --install`.

### 1.3.2. Qt, CMake, Ninja
  1. Go to https://www.qt.io/download-qt-installer and select the installer for Mac.
  2. Download the installation program and execute it.
  3. Install the following components:
     - Qt 5.15.2
        - [x] macOS
     - Developer and Designer Tools
        - [x] CMake
        - [x] Ninja

# 2. Build the DFU Host Tool

## 2.1. Update setenv.sh
  1. Set the QTDIR variable to the directory containing the compiler-specific subdirectory for Qt.
      > For example, "C:/Qt/5.15.2/msvc2019_64"
  2. Set the BOOST_ROOT variable to the directory containing Boost.
      > For example, "C:/boost-1.76.0-msvc141-x86_64-mswin"
  3. Set the CMAKE_COMMAND variable to the path to cmake executable.
      > For example, "C:/cmake-3.22.5-windows-x86_64/bin/cmake"
  4. Set the NINJA_COMMAND variable to the path to ninja executable.
      > For example, "C:/ninja-1.9.0-win/ninja"
  
  Note: On Windows, use forward slashes "/".

## 2.2. Run the build script
  1. On Windows, run `./build.bat`. The default generator is Visual Studio 16 2019.
  2. On Linux and MacOS, run `./build.sh`. The default generator is "Ninja Multi-Config".

  > Note: To select a different generator (e.g. "Xcode" on MacOS), use environment variable `GENERATOR_NAME`.
  Also, you can change the selected different build type (e.g. Debug instead of RelWithDebInfo), use environment variable `CONFIG`.
  By default, the script will build the `dfuh-tool_ALL` cmake target. You can specify other targets from CMakeLists.txt by providing an additional argument to the build script.
  The resulting custom command will be like this `env GENERATOR_NAME="Ninja Multi-Config" CONFIG=Debug ./build.sh dfuh-tool_BUILD_ONLY`.

## 2.3. Observe built executables
  The built executable will be under the `sample_code/tools_3.1/dfuh-tool` folder.

  Now, the dfuh-tool and dfuh-cli executables are ready to run.

## 2.4. Notes
  1. Now, the build system produces the same dfuh-tool as shipped with ModusToolbox™.
  2. The location of the build executables has changed since the previous release. Earlier, it was `sample_code/build/RelWithDebInfo` - now, it is `sample_code/tools_3.1/dfuh-tool`.
  3. Now, on MacOS, the dfuh-tool and dfuh-cli executables share the same dfuh-tool.app bundle. Earlier, they were located in separate bundles dfuh-tool.app and dfuh-cli.app. You can use dfuh-cli directly from the build folder using `./dfuh-cli` or find it inside the dfuh-tool.app bundle: `dfuh-tool.app/Contents/MacOS/dfuh-tool`.

## 2.5. Known issues
  1. XCode debugging of the dfuh-cli executable doesn't work. This should be fixed in next releases.
