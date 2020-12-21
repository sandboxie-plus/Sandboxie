:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
::
:: Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
:: Contact: http://www.qt-project.org/legal
::
:: This file is part of the Qt Solutions component.
::
:: $QT_BEGIN_LICENSE:BSD$
:: You may use this file under the terms of the BSD license as follows:
::
:: "Redistribution and use in source and binary forms, with or without
:: modification, are permitted provided that the following conditions are
:: met:
::   * Redistributions of source code must retain the above copyright
::     notice, this list of conditions and the following disclaimer.
::   * Redistributions in binary form must reproduce the above copyright
::     notice, this list of conditions and the following disclaimer in
::     the documentation and/or other materials provided with the
::     distribution.
::   * Neither the name of Digia Plc and its Subsidiary(-ies) nor the names
::     of its contributors may be used to endorse or promote products derived
::     from this software without specific prior written permission.
::
::
:: THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
:: "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
:: LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
:: A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
:: OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
:: SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
:: LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
:: DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
:: THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
:: (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
:: OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
::
:: $QT_END_LICENSE$
::
:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

@echo off

rem
rem "Main"
rem

if not "%1"=="" (
    if not "%1"=="-library" (
        call :PrintUsage
        goto EOF
    )
)

if exist config.pri. del config.pri
if "%1"=="-library" (
    echo Configuring to build this component as a dynamic library.
    echo SOLUTIONS_LIBRARY = yes > config.pri
)

echo .
echo This component is now configured.
echo .
echo To build the component library (if requested) and example(s),
echo run qmake and your make or nmake command.
echo .
echo To remove or reconfigure, run make (nmake) distclean.
echo .
goto EOF

:PrintUsage
echo Usage: configure.bat [-library]
echo .
echo -library: Build the component as a dynamic library (DLL). Default is to
echo           include the component source directly in the application.
echo           A DLL may be preferable for technical or licensing (LGPL) reasons.
echo .
goto EOF


:EOF
