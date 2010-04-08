# # # # # #
# Title: Imaginable
# Author: Kuzma Shapran
# Created: 2010-02-16
# Copyright: Kuzma Shapran <Kuzma.Shapran@gmail.com>
# License: GPLv3
# # # # # #

TARGET = imaginable

TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
QT += dbus
QT -= gui

BIN_DIR = bin
CONFIG(debug,debug|release):BUILD_DIR = /debug
else:BUILD_DIR = /release

CONFIG(debug,debug|release):DEFINES += _DEBUG

MAIN_DIR = ../$${BIN_DIR}$${BUILD_DIR}
DESTDIR = $${MAIN_DIR}/$${TEMPLATE}
OBJECTS_DIR = $${MAIN_DIR}/obj/$${TEMPLATE}/$${TARGET}
MOC_DIR = $${OBJECTS_DIR}
RCC_DIR = $${OBJECTS_DIR}
OUT_PWD = $${OBJECTS_DIR}

!exists(DESTDIR): system(mkdir -p $${DESTDIR})
!exists(OBJECTS_DIR): system(mkdir -p $${OBJECTS_DIR})

PATH_TO_PROJECT_ROOT = ..

INCLUDEPATH += \
	$${PATH_TO_PROJECT_ROOT}/include

SOURCES += \
	main.cpp \
	version.cpp \
	options.cpp \
	root_q.cpp \
	image.cpp \
	image_q.cpp

HEADERS += \
	main.hpp \
	version.hpp \
	options.hpp \
	root_q.hpp \
	image_q.hpp \
	$${PATH_TO_PROJECT_ROOT}/include/root.hpp \
	$${PATH_TO_PROJECT_ROOT}/include/image.hpp \
	$${PATH_TO_PROJECT_ROOT}/include/types.hpp \
	$${PATH_TO_PROJECT_ROOT}/include/plugin_iface.hpp

QMAKE_QDBUSXML2CPP = $$[QT_INSTALL_BINS]/qdbusxml2cpp -i $${PATH_TO_PROJECT_ROOT}/include/types.hpp

DBUS_ADAPTORS += \
	dbus_root_q.xml \
	dbus_image_q_main.xml \
	dbus_image_q_busy.xml

OTHER_FILES += \
	version-minor.inl \
	version-major.inl \
	version-label.inl \
	$${DBUS_ADAPTORS}


autoversioning.target = version.cpp
autoversioning.commands = "@bash $${PATH_TO_PROJECT_ROOT}/tools/autoversioning.sh"
autoversioning.depends = autoversioning2

autoversioning2.commands = "@echo Autoversioning ..."

QMAKE_EXTRA_TARGETS += autoversioning autoversioning2
