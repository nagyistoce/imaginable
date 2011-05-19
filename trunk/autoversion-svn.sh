#!/bin/bash

##############
##
## Project:      Imaginable
## File info:    $Id$
## Author:       Copyright (C) 2009-2011 Kuzma Shapran <Kuzma.Shapran@gmail.com>
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

VERSION_FILE="version.cpp"
REVISION_FILE="version-revision.auto.inl"
NUMBER_FILE="version-number.auto.inl"
OLD_NUMBER_FILE="version-number.old.inl"
TIME_FILE="version-time.auto.inl"

WD="$(pwd)"
cd "$1"

OLD_REVISION="$(cat "${REVISION_FILE}")"
NEW_REVISION="\"$(svnversion)\""
echo "Revision: ${NEW_REVISION}"
echo "${NEW_REVISION}" > "${REVISION_FILE}"

if [[ "-${OLD_REVISION}" != "-${NEW_REVISION}" ]]
then
	(( NEW_NUMBER = 1 ))
else
	OLD_NUMBER="$(cat "${NUMBER_FILE}")"
	(( NEW_NUMBER = OLD_NUMBER + 1 ))
fi
echo "Build number: ${NEW_NUMBER}"
echo "${NEW_NUMBER}" > "${NUMBER_FILE}"

BUILD_TIME="$(date '+%s')"
echo "Build time: ${BUILD_TIME} ($(date -d "@${BUILD_TIME}"))"
echo "${BUILD_TIME}" > "${TIME_FILE}"

touch "${VERSION_FILE}"

cd "${WD}"
