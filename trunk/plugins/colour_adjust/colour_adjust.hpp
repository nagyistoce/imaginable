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

#ifndef IMAGINABLE__PLUGINS__COLOUR_ADJUST__COLOUR_ADJUST__INCLUDED
#define IMAGINABLE__PLUGINS__COLOUR_ADJUST__COLOUR_ADJUST__INCLUDED


#include "version.hpp"

#include <plugin_iface.hpp>

#include "plugin_types.hpp"


class PluginColourAdjust : public QObject, PluginInterface
{
Q_OBJECT
Q_INTERFACES(PluginInterface)
public:
	PluginColourAdjust(void);
	~PluginColourAdjust() {}

	QString name   (void) const { return "colour_adjust"; }
	QString version(void) const { return QString::fromAscii(version::full_string()); }

signals:
	void setPercent(double);

public slots:
	QString errorCodeToString(uint errorCode) const;

	uint invertAll(qulonglong);
	uint invert(qulonglong,int);

	uint gammaAll(qulonglong,double);
	uint gamma(qulonglong,int,double);

	uint curveAll(qulonglong,QAdjustPoint);
	uint curve(qulonglong,int,QAdjustPoint);

private:
	void do_invertAll(qulonglong,Image*);
	void do_invertPlain(qulonglong,Image*,int);
	void do_invert(qulonglong,Image*,int,double,double);

	void do_gammaAll(qulonglong,Image*,double);
	void do_gammaPlain(qulonglong,Image*,int,double);
	void do_gamma(qulonglong,Image*,int,double,double,double);

	void do_curveAll(qulonglong,Image*,QAdjustPoint);
	void do_curvePlain(qulonglong,Image*,int,QAdjustPoint);
	void do_curve(qulonglong,Image*,int,QAdjustPoint,double,double);

	enum
	{
		/**/CODE_NO_COLOUR_PLANE = Core::CODE__CUSTOM
	};
};

#endif // IMAGINABLE__PLUGINS__COLOUR_ADJUST__COLOUR_ADJUST__INCLUDED
