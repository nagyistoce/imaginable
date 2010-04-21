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

#ifndef IMAGINABLE__PLUGINS__ROTATE__ROTATE__INCLUDED
#define IMAGINABLE__PLUGINS__ROTATE__ROTATE__INCLUDED


#include "version.hpp"

#include <plugin_iface.hpp>


class PluginRotate : public QObject, PluginInterface
{
Q_OBJECT
Q_INTERFACES(PluginInterface)
public:
	PluginRotate(void);
	~PluginRotate() {}

	QString name   (void) const { return "rotate"; }
	QString version(void) const { return QString::fromAscii(version::full_string()); }

signals:
	void setPercent(double);

public slots:
	QString errorCodeToString(uint errorCode) const;

	qulonglong rotate(qulonglong,double);

	uint lastErrorCode(qulonglong);

private:
	typedef QMap<qulonglong,uint> lastErrorCodes_t;
	lastErrorCodes_t m_lastErrorCodes;

	void do_rotate(qulonglong,Image*,qulonglong,Image*,double);
};

#endif // IMAGINABLE__PLUGINS__ROTATE__ROTATE__INCLUDED
