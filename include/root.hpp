/*************
**
** Project:      Imaginable
** File info:    $Id$
** Author:       Copyright (C) 2009,2010 Kuzma Shapran <Kuzma.Shapran@gmail.com>
** License:      GPLv3
**
**  This file is part of Imaginable.
**
**  Imaginable is free software: you can redistribute it and/or modify
**  it under the terms of the GNU General Public License as published by
**  the Free Software Foundation, either version 3 of the License, or
**  (at your option) any later version.
**
**  Imaginable is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
**
**  You should have received a copy of the GNU General Public License
**  along with Imaginable.  If not, see <http://www.gnu.org/licenses/>.
**
*************/

#ifndef IMAGINABLE__ROOT__INCLUDED
#define IMAGINABLE__ROOT__INCLUDED


#include <QtCore/QtGlobal>

#include <sys/syslog.h> // for log levels


class Image;

class Root
{
public:
	Root(void) {}
	virtual ~Root() {}

	virtual Image* image(qulonglong) =0;
	virtual bool hasImage(qulonglong) const =0;

	virtual void pluginMessage(int level,QString plugin,qulonglong Id,QString message) const =0;
};

#endif // IMAGINABLE__ROOT__INCLUDED
