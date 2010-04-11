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

#ifndef IMAGINABLE__PLUGINS__QTIO__QTIO__INCLUDED
#define IMAGINABLE__PLUGINS__QTIO__QTIO__INCLUDED


#include "version.hpp"

#include <plugin_iface.hpp>

class PluginQTIO : public QObject, PluginInterface
{
Q_OBJECT
Q_INTERFACES(PluginInterface)
public:
	PluginQTIO(void);
	~PluginQTIO() {}

	bool init(Root*);
	QString name   (void) const { return "/qtio"; }
	QString version(void) const { return QString::fromAscii(version::full_string()); }

signals:
	void lock_percent(double);

public slots:
	uint loadTo(QString filename,qulonglong Id);
	qulonglong load(QString);
	uint saveWithQuality(qulonglong,QString,int);
	uint save(qulonglong,QString);

};

#endif // IMAGINABLE__PLUGINS__QTIO__QTIO__INCLUDED
