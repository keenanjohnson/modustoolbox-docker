@REM Copyright 2018-2023 Cypress Semiconductor Corporation (an Infineon company)
@REM SPDX-License-Identifier: Apache-2.0
@REM
@REM Licensed under the Apache License, Version 2.0 (the "License");
@REM you may not use this file except in compliance with the License.
@REM You may obtain a copy of the License at
@REM
@REM     http://www.apache.org/licenses/LICENSE-2.0
@REM
@REM Unless required by applicable law or agreed to in writing, software
@REM distributed under the License is distributed on an "AS IS" BASIS,
@REM WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
@REM See the License for the specific language governing permissions and
@REM limitations under the License.

:: This script loads Visual Studio Development Environment and runs build.sh

:: Check if bash interpreter is found in PATH
@WHERE bash >NUL 2>&1
:: bash not found: prepend C:\cygwin64\bin to PATH and try again
@IF ERRORLEVEL 1 SET PATH=C:\cygwin64\bin;%PATH%
@WHERE bash
@IF ERRORLEVEL 1 @(
  @ECHO ERROR: bash interpreter not found: install Cygwin to C:\cygwin64
  @GOTO :ERROR
)

@SET VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe
@IF NOT EXIST "%VSWHERE%" @(
  @ECHO ERROR: Microsoft Visual Studio Build Tools are required but not found
  @GOTO :ERROR
)
@REM Note: ^[16.0^,17.0^) is required to always select VS2019 instead of any newer version
@FOR /F "usebackq tokens=* delims=: " %%i IN (`"%VSWHERE%" -nologo -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -version ^[16.0^,17.0^) -latest -property installationPath`) DO @(
  SET VSTOOLS=%%i\Common7\Tools
)
@IF NOT EXIST "%VSTOOLS%" @(
  @ECHO ERROR: Visual Studio Tools directory cannot be determined
  @GOTO :ERROR
)

@REM Avoid multiple sourcing of vsdevcmd.bat
@IF NOT DEFINED VCToolsInstallDir @(
  @REM "Set amd64 architecture for the produced binaries
  @ECHO "%VSTOOLS%\vsdevcmd.bat" -arch=amd64
  CALL "%VSTOOLS%\vsdevcmd.bat" -arch=amd64
)

bash %~dp0build.sh %*
@IF ERRORLEVEL 1 GOTO :ERROR
@GOTO :EOF

:ERROR
@ECHO.
@ECHO FAILURE
@exit /b 1
