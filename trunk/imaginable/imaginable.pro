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

TEMPLATE = lib

SOURCES += \
	image.cpp \
	point.cpp \
	version.cpp \
	io_pnm_loader.cpp \
	io_pam_saver.cpp \
	io_qt.cpp \
	tools_maximum.cpp \
	tools_gamma.cpp \
	tools_rotate.cpp \
	tools_crop.cpp \
	tools_colourspace.cpp \
	tools_blur.cpp \
	tools_tonemap.cpp

OTHER_FILES += \
	version-major.inl \
	version-minor.inl \
	version-label.inl

HEADERS += \
	image.hpp \
	point.hpp \
	version.hpp \
	io_pnm_loader.hpp \
	io_pam_saver.hpp \
	io_qt.hpp \
	tools.hpp \
	tools_maximum.hpp \
	tools_gamma.hpp \
	tools_rotate.hpp \
	tools_crop.hpp \
	tools_colourspace.hpp \
	tools_blur.hpp \
	tools_tonemap.hpp


autoversioning.commands = "@bash $$dirname(_PRO_FILE_PWD_)/autoversion-svn.sh $$_PRO_FILE_PWD_"
autoversioning.depends = autoversioning_echo
PRE_TARGETDEPS = autoversioning

autoversioning_echo.commands = "@echo Autoversioning ..."

QMAKE_EXTRA_TARGETS += autoversioning autoversioning_echo
