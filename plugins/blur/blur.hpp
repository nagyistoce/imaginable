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

#ifndef IMAGINABLE__PLUGINS__BLUR__BLUR__INCLUDED
#define IMAGINABLE__PLUGINS__BLUR__BLUR__INCLUDED


#include "version.hpp"

#include <plugin_iface.hpp>


class PluginBlur : public QObject, PluginInterface
{
Q_OBJECT
Q_INTERFACES(PluginInterface)
public:
	PluginBlur(void);
	~PluginBlur() {}

	QString name   (void) const { return "blur"; }
	QString version(void) const { return QString::fromAscii(version::full_string()); }

signals:
	void setPercent(double);

public slots:
	QString errorCodeToString(uint errorCode) const;

	uint boxAll       (qulonglong,uint);
	uint boxAllPercent(qulonglong,uint);
	uint box          (qulonglong,int,uint);
	uint boxPercent   (qulonglong,int,uint);

	uint frameAll       (qulonglong,uint,uint);
	uint frame          (qulonglong,int,uint,uint);

private:
	void do_boxAll  (qulonglong,Image*,int);
	void do_boxPlain(qulonglong,Image*,int,int);
	void do_box     (qulonglong,Image*,int,int,double,double);

	void do_frameAll  (qulonglong,Image*,int,int);
	void do_framePlain(qulonglong,Image*,int,int,int);
	void do_frame     (qulonglong,Image*,int,int,int,double,double);

	enum
	{
		/**/CODE_NO_COLOUR_PLANE = Core::CODE__CUSTOM
		,   CODE_IMAGE_IS_EMPTY
	};
};

#endif // IMAGINABLE__PLUGINS__BLUR__BLUR__INCLUDED
