#-------------------------------------------------
#
# Project created by QtCreator 2014-07-30T15:07:02
#
#-------------------------------------------------

QT       += core gui svg xml network # opengl
QTPLUGIN += qsvg qico

# win32:	QTPLUGIN += qsvgicon

# DEFINES += _DETALHE_MOTOR=1
# DEFINES += _DETALHE_MOTOR_APPLIED=1

# include(.. /qtserialport / qtserialport.p ri)			# trabalho futuro (tirando os espaços)
include(../qextserialport/src/qextserialport.pri)

contains(DEFINES, _DETALHE_MOTOR_APPLIED) | contains(DEFINES, _DETALHE_MOTOR) | CONFIG(debug)	{
	include(../LogNetSender/LogServer.pri)
}

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = TestaApplied
TEMPLATE = app


SOURCES += \
	main.cpp\
	QtSerialTest.cpp \
	CDispositivoMotorApplied.cpp \
	IDispositivoMotor.cpp

HEADERS  += \
	QtSerialTest.h \
	../QtCliche/NomesDispositivos.h \
	../QtCliche/Property.h \
	CDispositivoMotorApplied.h \
	IDispositivoMotor.h

FORMS    += QtSerialTest.ui

UI_DIR += ./GeneratedFiles
RCC_DIR += ./GeneratedFiles
MOC_DIR += ./GeneratedFiles

INCLUDEPATH += ../QtCliche ./GeneratedFiles

release {
	DESTDIR = ./release
	OBJECTS_DIR += ./GeneratedFiles/release
	INCLUDEPATH += ./GeneratedFiles/release

	DEFINES += NDEBUG=1
	DEFINES += _NDEBUG=1

	win32:	DEFINES += _TTY_WIN_ # QWT_DLL QT_DLL
}

debug	{
	DESTDIR = ./debug
	OBJECTS_DIR += ./GeneratedFiles/debug
	INCLUDEPATH += ./GeneratedFiles/debug

	DEFINES += _DEBUG_DISPOSITIVO_MOTOR
	DEFINES += _DETALHE_MOTOR_APPLIED
	DEFINES += _DEBUG

	win32:	DEFINES += _TTY_WIN_ QWT_DLL QT_DLL
}

unix:	DEFINES += _TTY_POSIX_ _TTY_NOWARN_

DEFINES += QTSERIALTEST

RESOURCES += \
	QtSerialTest.qrc

win32: RC_FILE = QtSerialTest.rc
OTHER_FILES += QtSerialTest.rc

win32 {
	defineReplace(Revisions)	{
		variable = $$1
		ALLFILES = $$eval($$variable)
		NUMBERS = $$system(set LANG=en_US && svn info | sed --quiet /Rev:/p | cut -d: -f2 )
		return ($$NUMBERS)
	}
}

unix {
	defineReplace(Revisions)	{
		NUMBERS = $$system(LC_ALL=C svn info | grep "Rev:" | cut -d\" \"   -f4)
		# message($$NUMBERS)
		return ($$NUMBERS)
	}
}

unix {
	QMAKE_CFLAGS_DEBUG -= -g
	QMAKE_CXXFLAGS_DEBUG -= -g
	QMAKE_CFLAGS_DEBUG += -O0 -g -ggdb
	QMAKE_CXXFLAGS_DEBUG += -O0 -g -ggdb
	QMAKE_CFLAGS_RELEASE -= -O2
	QMAKE_CXXFLAGS_RELEASE -= -O2
	QMAKE_CFLAGS_RELEASE += -O3 -mtune=native -march=native -mfpmath=sse -momit-leaf-frame-pointer -msahf -mcx16 -pipeB
	QMAKE_CXXFLAGS_RELEASE += -O3 -mtune=native -march=native -mfpmath=sse -momit-leaf-frame-pointer -msahf -mcx16 -pipe
}

REV_MAJOR_CODE = 0
REV_MINOR_CODE = 9
REV_PATCH_CODE = 16

VER = $$REV_MAJOR_CODE $$REV_MINOR_CODE $$REV_PATCH_CODE $$Revisions()
VERSAO = $$join(VER, ".")

DEFINES += \
	REV_SUBVN_CODE=$$Revisions() \
	REV_PATCH_CODE=$$REV_PATCH_CODE \
	REV_MINOR_CODE=$$REV_MINOR_CODE \
	REV_MAJOR_CODE=$$REV_MAJOR_CODE \
	REV_CODE=\\\"$$VERSAO\\\" \
	APP_VERSION=\\\"$$VERSAO\\\" \
	APP_NAME=\"$$TARGET\"

# message($$DEFINES)
message($$DESTDIR"/$$TARGET v."$$VERSAO"  -  Qt "$$QT_VERSION"   -   Compilador: "$$QMAKE_COMPILER)
