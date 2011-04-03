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

#ifndef IMAGINABLE__MODULE_INTERFACE__INCLUDED
#define IMAGINABLE__MODULE_INTERFACE__INCLUDED


#include <QtCore/QtPlugin>
#include <QtCore/QString>
#include <QtCore/QSet>

#include <function.hpp>


class Module
{
public:
	Module(void)
	{}
	virtual ~Module()
	{}

	virtual QString resource_type (void) const =0;

	virtual QSet<FunctionDescription> depends_on (void) const =0;

	virtual QSet<FunctionDescription> provides (void) const =0;

	virtual QSharedPointer<CallResult> call (QString name,const Arguments &input,Arguments &output) =0;
};

Q_DECLARE_INTERFACE(Module,"imaginable.Module/1.0")

#endif // IMAGINABLE__MODULE_INTERFACE__INCLUDED
