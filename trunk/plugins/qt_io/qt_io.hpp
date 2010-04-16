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

#ifndef IMAGINABLE__PLUGINS__QT_IO__QT_IO__INCLUDED
#define IMAGINABLE__PLUGINS__QT_IO__QT_IO__INCLUDED


#include "version.hpp"

#include <plugin_iface.hpp>

class PluginQT_IO : public QObject, PluginInterface
{
Q_OBJECT
Q_INTERFACES(PluginInterface)
public:
	PluginQT_IO(void);
	~PluginQT_IO() {}

	bool init(Root*);
	QString name   (void) const { return "/qt_io"; }
	QString version(void) const { return QString::fromAscii(version::full_string()); }

public slots:
	QString errorCodeToString(uint) const;

	uint load(QString filename,qulonglong Id);
	qulonglong loadNew(QString);
	uint saveWithQuality(qulonglong,QString,int);
	uint save(qulonglong,QString);

private:

	enum
	{
		/**/CODE_NO_SRC_FILE               = Root::CODE__CUSTOM
		,   CODE_INVALID_SRC_FILE
		,   CODE_NOT_SUPPORTED_COLOURSPACE
		,   CODE_DST_FILE_EXIST
		,   CODE_INVALID_DST_FILE
	};
};

#endif // IMAGINABLE__PLUGINS__QT_IO__QT_IO__INCLUDED
