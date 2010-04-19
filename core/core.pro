##############
##
## Project:      Imaginable
## File info:    $Id$
## Author:       Copyright (C) 2009,2010 Kuzma Shapran <Kuzma.Shapran@gmail.com>
## License:      GPLv3
##
##  This file is part of Imaginable.
##
##  Imaginable is free software: you can redistribute it and/or modify
##  it under the terms of the GNU General Public License as published by
##  the Free Software Foundation, either version 3 of the License, or
##  (at your option) any later version.
##
##  Imaginable is distributed in the hope that it will be useful,
##  but WITHOUT ANY WARRANTY; without even the implied warranty of
##  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
##  GNU General Public License for more details.
##
##  You should have received a copy of the GNU General Public License
##  along with Imaginable.  If not, see <http://www.gnu.org/licenses/>.
##
##############

TARGET = imaginable

TEMPLATE = app
CONFIG += console
QT += dbus
QT -= gui

BIN_DIR = bin
CONFIG(debug,debug|release):BUILD_DIR = /debug
else:BUILD_DIR = /release

CONFIG(debug,debug|release):DEFINES += _DEBUG

PATH_TO_PROJECT_ROOT = ..

MAIN_DIR = $${PATH_TO_PROJECT_ROOT}/$${BIN_DIR}$${BUILD_DIR}
DESTDIR = $${MAIN_DIR}/$${TEMPLATE}
OBJECTS_DIR = $${MAIN_DIR}/obj/$${TEMPLATE}/$${TARGET}
MOC_DIR = $${OBJECTS_DIR}
RCC_DIR = $${OBJECTS_DIR}
OUT_PWD = $${OBJECTS_DIR}

!exists(DESTDIR):system(mkdir -p $${DESTDIR})
!exists(OBJECTS_DIR):system(mkdir -p $${OBJECTS_DIR})

INCLUDEPATH += \
	$${PATH_TO_PROJECT_ROOT}/include \
	$${PATH_TO_PROJECT_ROOT}/common

SOURCES += \
	main.cpp \
	version.cpp \
	../common/options.cpp \
	root_q.cpp \
	image.cpp \
	image_q.cpp

HEADERS += \
	main.hpp \
	version.hpp \
	../common/options.hpp \
	root_q.hpp \
	image_q.hpp \
	$${PATH_TO_PROJECT_ROOT}/include/root.hpp \
	$${PATH_TO_PROJECT_ROOT}/include/image.hpp \
	$${PATH_TO_PROJECT_ROOT}/include/types.hpp \
	$${PATH_TO_PROJECT_ROOT}/include/plugin_iface.hpp

QMAKE_QDBUSXML2CPP = \
	$$[QT_INSTALL_BINS]/qdbusxml2cpp -i $${PATH_TO_PROJECT_ROOT}/include/types.hpp

DBUS_ADAPTORS += \
	dbus_root_q.xml \
	dbus_image_q_main.xml \
	dbus_image_q_busy.xml

OTHER_FILES += \
	version-minor.inl \
	version-major.inl \
	version-label.inl \
	$${DBUS_ADAPTORS} \
	$${PATH_TO_PROJECT_ROOT}/tools/autoversioning.sh


autoversioning.target = version.cpp
autoversioning.commands = "@bash $${PATH_TO_PROJECT_ROOT}/tools/autoversioning.sh"
autoversioning.depends = autoversioning2

autoversioning2.commands = "@echo Autoversioning ..."

QMAKE_EXTRA_TARGETS += autoversioning autoversioning2