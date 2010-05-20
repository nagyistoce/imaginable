/*************
**
** Project:      Imaginable
** File info:    $Id: types.hpp 21 2010-04-21 09:08:38Z Kuzma.Shapran@gmail.com $
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

#ifndef IMAGINABLE__PLUGINS__COLOUR_ADJUST__TYPES__INCLUDED
#define IMAGINABLE__PLUGINS__COLOUR_ADJUST__TYPES__INCLUDED


#include <QtCore/QMetaType>
#include <QtCore/QHash>
#include <QtCore/QString>


typedef QHash<ushort,ushort> QAdjustPoint;
Q_DECLARE_METATYPE(QAdjustPoint);

typedef QHash<uint,QString> QFunctionList;
Q_DECLARE_METATYPE(QFunctionList);


#endif // IMAGINABLE__PLUGINS__COLOUR_ADJUST__TYPES__INCLUDED
