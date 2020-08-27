@echo off
set LANG=en_US
del /q TestaApplied*.exe
del /q TestaApplied*.upx

copy /y IDispositivoMotor.cpp ..\TestaApplied\
copy /y IDispositivoMotor.h ..\TestaApplied\
copy /y CDispositivoMotorApplied.cpp ..\TestaApplied\
copy /y CDispositivoMotorApplied.h ..\TestaApplied\

set PATH=C:\Qt\4.8.5.static\bin;%QT_CREATOR_PATH%;C:\WINDOWS\system32;C:\WINDOWS;C:\WINDOWS\System32\Wbem;"C:\Program Files\Subversion\bin";C:\MinGW\bin;C:\MinGW\MSYS\1.0\bin;
rem set PATH=%QT_PATH%;%QT_CREATOR_PATH%;C:\WINDOWS\system32;C:\WINDOWS;C:\WINDOWS\System32\Wbem;"C:\Program Files\Subversion\bin";C:\MinGW\bin;C:\MinGW\MSYS\1.0\bin;
call "C:\Program Files\Microsoft Visual Studio 8\VC\vcvarsall.bat"
qmake -r -spec win32-msvc2005 "CONFIG-=debug debug_and_release debug_and_release_target release" "CONFIG+=release static staticlibs" && jom -j2 -l && move release\TestaApplied.exe .\TestaApplied.exe && call \UPX.bat TestaApplied.exe && call distclean.bat
rem qmake -r -spec win32-msvc2005 "CONFIG-=debug_and_release release debug" "CONFIG+=release"                                                                         && jom -j2 -l && move release\TestaApplied.exe .\TestaApplied.exe && call \UPX.bat TestaApplied.exe && call distclean.bat