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

#ifndef IMAGINABLE__PLUGINS__OVERLAY__OVERLAY__INCLUDED
#define IMAGINABLE__PLUGINS__OVERLAY__OVERLAY__INCLUDED


#include "version.hpp"

#include <plugin_iface.hpp>


class PluginOverlay : public QObject, PluginInterface
{
Q_OBJECT
Q_INTERFACES(PluginInterface)
public:
	PluginOverlay(void);
	~PluginOverlay() {}

	QString name   (void) const { return "overlay"; }
	QString version(void) const { return QString::fromAscii(version::full_string()); }

signals:
	void setPercent(double);

public slots:
	QString errorCodeToString(uint errorCode) const;

	uint difference(qulonglong,int,int,int);
	uint alphaBlend(qulonglong,int,int,int,int);

private:
	void do_difference(qulonglong,Image*,int,int,int);
	void do_alphaBlend(qulonglong,Image*,int,int,int,int);

	enum
	{
		/**/CODE_NO_SRC_COLOUR_PLANE_1   = Core::CODE__CUSTOM
		,   CODE_NO_SRC_COLOUR_PLANE_2
		,   CODE_NO_SRC_COLOUR_PLANE_3
		,   CODE_DST_COLOUR_PLANE_EXISTS
	};
};

#endif // IMAGINABLE__PLUGINS__OVERLAY__OVERLAY__INCLUDED
