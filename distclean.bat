@echo off
set PATH=%QT_PATH%;%QT_CREATOR_PATH%;C:\WINDOWS\system32;C:\WINDOWS;C:\WINDOWS\System32\Wbem;"C:\Program Files\Subversion\bin";C:\MinGW\bin;C:\MinGW\MSYS\1.0\bin;
call "C:\Program Files\Microsoft Visual Studio 8\VC\vcvarsall.bat"
qmake && jom -j2 -l distclean
del /f /q /s release\*.*
del /f /q /s debug\*.*
del /f /q /s GeneratedFiles\*.*
del /f /q ui_*.h
del /f /q moc_*.cpp
del /f /q qrc_*.cpp
rmdir debug
rmdir release
rmdir GeneratedFiles\debug
rmdir GeneratedFiles\release
rmdir GeneratedFiles
