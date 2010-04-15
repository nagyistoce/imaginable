/*************
**
** Project:      Imaginable
** File info:    $Id: clone.hpp 16 2010-04-13 10:59:29Z Kuzma.Shapran@gmail.com $
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

#ifndef IMAGINABLE__PLUGINS__CLONE__CLONE__INCLUDED
#define IMAGINABLE__PLUGINS__CLONE__CLONE__INCLUDED


#include "version.hpp"

#include <plugin_iface.hpp>


class PluginPlanes : public QObject, PluginInterface
{
Q_OBJECT
Q_INTERFACES(PluginInterface)
public:
	PluginPlanes(void);
	~PluginPlanes() {}

	bool init(Root*);
	QString name   (void) const { return "/planes"; }
	QString version(void) const { return QString::fromAscii(version::full_string()); }

signals:
	void lock_percent(double);

public slots:
	QString errorCodeToString(uint errorCode) const;

	uint copy(qulonglong fromId,qulonglong toId,int fromPlane,int toPlane);

	uint rgb2luma(qulonglong);

	uint rgb2hsv(qulonglong);
	uint hsv2rgb(qulonglong);

	uint rgb2hsl(qulonglong);
	uint hsl2rgb(qulonglong);

	uint hsv2hsl(qulonglong);
	uint hsl2hsv(qulonglong);

	uint uncompressHue(qulonglong);
	uint compressHue  (qulonglong);


private:
	enum
	{
		/**/CODE_PLANES_DONT_DIFFER       = Root::CODE__CUSTOM
		,   CODE_NO_SOURCE_PLANE
		,   CODE_DESTINATION_PLANE_EXISTS
		,   CODE_SIZES_DIFFER
	};
};

#endif // IMAGINABLE__PLUGINS__CLONE__CLONE__INCLUDED
