# # # # # #
# Title: Imaginable::clone
# Author: Kuzma Shapran
# Created: 2010-03-27
# Copyright: Kuzma Shapran <Kuzma.Shapran@gmail.com>
# License: GPLv3
# # # # # #

TARGET = clone

TEMPLATE = lib
CONFIG += plugin
CONFIG -= app_bundle
QT += dbus
QT -= gui

BIN_DIR = bin
CONFIG(debug,debug|release):BUILD_DIR = /debug
else:BUILD_DIR = /release

CONFIG(debug,debug|release):DEFINES += _DEBUG

MAIN_DIR = ../../$${BIN_DIR}$${BUILD_DIR}
DESTDIR = $${MAIN_DIR}/$${TEMPLATE}
OBJECTS_DIR = $${MAIN_DIR}/obj/$${TEMPLATE}/$${TARGET}
MOC_DIR = $${OBJECTS_DIR}
RCC_DIR = $${OBJECTS_DIR}
OUT_PWD = $${OBJECTS_DIR}

!exists(DESTDIR): system(mkdir -p $${DESTDIR})
!exists(OBJECTS_DIR): system(mkdir -p $${OBJECTS_DIR})

PATH_TO_PROJECT_ROOT = ../..

INCLUDEPATH += \
	$${PATH_TO_PROJECT_ROOT}/include

SOURCES += \
	version.cpp \
	plugin_clone.cpp

HEADERS += \
	version.hpp \
	plugin_clone.hpp

QMAKE_QDBUSXML2CPP = $$[QT_INSTALL_BINS]/qdbusxml2cpp -i $${PATH_TO_PROJECT_ROOT}/include/types.hpp

DBUS_ADAPTORS += \
	dbus_plugin_clone.xml

OTHER_FILES += \
	version-minor.inl \
	version-major.inl \
	version-label.inl \
	$${DBUS_ADAPTORS}


autoversioning.target = version.cpp
autoversioning.commands = "bash $${PATH_TO_PROJECT_ROOT}/tools/autoversioning.sh"
autoversioning.depends = autoversioning2

autoversioning2.commands = @echo Autoversioning

QMAKE_EXTRA_TARGETS += autoversioning autoversioning2
