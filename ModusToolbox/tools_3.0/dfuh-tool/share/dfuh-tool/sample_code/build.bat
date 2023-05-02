@REM Copyright 2018-2022 Cypress Semiconductor Corporation (an Infineon company)
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

@SETLOCAL
@IF "%1" == "" (SET TARGET=dfuh-tool_ALL) ELSE (SET TARGET=%1)

@REM The following dependencies must be installed manually
@SET VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe
@IF NOT EXIST "%VSWHERE%" @(
  @ECHO ERROR: Microsoft Visual Studio Build Tools are requried but not found
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

@IF NOT DEFINED CONFIG SET CONFIG=RelWithDebInfo

@IF NOT EXIST build MKDIR build
@CD build
@IF NOT EXIST reports MKDIR reports

@REM "VC++ 2019 version 16.10 v14.29 toolset" must be installed in Visual Studio Installer
@CALL "%VSTOOLS%/vsdevcmd.bat" -arch=amd64
@IF ERRORLEVEL 1 GOTO :ERROR

@REM Developer build: generate Visual Studio projects
@REM CI build: generate Ninja build rules
@REM Developers can override generator for testing:
@REM SET GENERATOR_NAME=Ninja
@REM build.bat

@IF NOT DEFINED GENERATOR_NAME SET GENERATOR_NAME=Visual Studio 16 2019
@ECHO GENERATOR_NAME=%GENERATOR_NAME%
@IF "%GENERATOR_NAME%" == "Ninja" @(
  SET CC=cl.exe
  SET CXX=cl.exe
  SET CMAKE_CMD_EXTRA=-DCMAKE_EXPORT_COMPILE_COMMANDS=ON
)

cmake -G "%GENERATOR_NAME%" %CMAKE_CMD_EXTRA% -DCMAKE_BUILD_TYPE=%CONFIG% ..
@IF ERRORLEVEL 1 GOTO :ERROR
cmake --build . --config %CONFIG% --target %TARGET%
@IF ERRORLEVEL 1 GOTO :ERROR
@GOTO :SUCCESS

:ERROR
@ECHO.
@ECHO FAILURE
@exit /b 1

:SUCCESS
@ECHO.
@ECHO SUCCESS

@REM PAUSE
@ENDLOCAL

