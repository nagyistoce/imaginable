##############
##
## Project:      Imaginable
## File info:    $Id$
## Author:       Copyright (C) 2009 - 2011 Kuzma Shapran <Kuzma.Shapran@gmail.com>
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


TARGET = tonemap_gui

DESTDIR = ../bin

TEMPLATE = app
QT += core gui


SOURCES += \
	main.cpp\
	version.cpp \
	mainwindow.cpp

HEADERS += \
	version.hpp \
	mainwindow.hpp

FORMS += \
	mainwindow.ui

OTHER_FILES += \
	version-major.inl \
	version-minor.inl \
	version-label.inl


INCLUDEPATH += ..


LIBS += \
	-L../lib \
	-limaginable

PRE_TARGETDEPS += ../lib/libimaginable.so


autoversioning.commands = "@bash $$dirname(_PRO_FILE_PWD_)/autoversion-svn.sh $$_PRO_FILE_PWD_"
autoversioning.depends = autoversioning_echo
PRE_TARGETDEPS += autoversioning

autoversioning_echo.commands = "@echo Autoversioning ..."

QMAKE_EXTRA_TARGETS += autoversioning autoversioning_echo

RESOURCES += \
    tonemap_gui.qrc
