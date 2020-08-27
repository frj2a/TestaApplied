@echo off
rem set PATH=C:\Qt\4.8.5.static\bin;%QT_CREATOR_PATH%;C:\WINDOWS\system32;C:\WINDOWS;C:\WINDOWS\System32\Wbem;"C:\Program Files\Subversion\bin";C:\MinGW\bin;C:\MinGW\MSYS\1.0\bin;
set PATH=%QT_PATH%;C:\Qt\qtcreator-2.8.0\bin;C:\WINDOWS\system32;C:\WINDOWS;C:\WINDOWS\System32\Wbem;"C:\Program Files\Subversion\bin";C:\MinGW\bin;C:\MinGW\MSYS\1.0\bin;
call "C:\Program Files\Microsoft Visual Studio 8\VC\vcvarsall.bat"
jom -j2 -l clean