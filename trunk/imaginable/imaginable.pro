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


TARGET = imaginable

DESTDIR = ../lib

TEMPLATE = lib

QT += core gui


SOURCES += \
	image.cpp \
	point.cpp \
	version.cpp \
	io_pnm_loader.cpp \
	io_pam_saver.cpp \
	io_qt.cpp \
	exception.cpp \
	notifier.cpp \
	time.cpp \
	maximum.cpp \
	gamma.cpp \
	rotate.cpp \
	crop.cpp \
	auto_crop.cpp \
	colourspace.cpp \
	blur.cpp \
	tonemap_global.cpp \
	tonemap_local_average.cpp \
	tonemap_local_minmax.cpp \
	scale.cpp \
	roof.cpp

HEADERS += \
	image.hpp \
	point.hpp \
	version.hpp \
	io_pnm_loader.hpp \
	io_pam_saver.hpp \
	io_qt.hpp \
	exception.hpp \
	notifier.hpp \
	time.hpp \
	maximum.hpp \
	gamma.hpp \
	rotate.hpp \
	crop.hpp \
	auto_crop.hpp \
	colourspace.hpp \
	blur.hpp \
	tonemap.hpp \
	scale.hpp \
	roof.hpp

OTHER_FILES += \
	version-major.inl \
	version-minor.inl \
	version-label.inl



win32:CYGWIN_PATH = C:/cygwin/bin
win32:BOOST_PATH = C:/boost_1_47_0



win32:INCLUDEPATH += $$BOOST_PATH


win32:{
	autoversioning.BATNAME = $$dirname(_PRO_FILE_PWD_)/autoversion-svn.bat
	autoversioning.commands = "@cmd /c $$replace(autoversioning.BATNAME, /, \\) $$replace(CYGWIN_PATH, /, \\)"
}else{
	autoversioning.commands = "bash"
}
autoversioning.commands += " $$dirname(_PRO_FILE_PWD_)/autoversion-svn.sh $$_PRO_FILE_PWD_"
autoversioning.depends = autoversioning_echo
PRE_TARGETDEPS += autoversioning

autoversioning_echo.commands = "@echo Autoversioning ..."

QMAKE_EXTRA_TARGETS += autoversioning autoversioning_echo
