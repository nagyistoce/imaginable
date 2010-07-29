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

#ifndef IMAGINABLE__PLUGINS__COLOUR_ADJUST__LINEAR__INCLUDED
#define IMAGINABLE__PLUGINS__COLOUR_ADJUST__LINEAR__INCLUDED


#include <QtCore/QMap>

#include "curve_function.hpp"
#include "plugin_types.hpp"


class Linear : public CurveFunction
{
public:
	Linear(Image::Pixel,Image::Pixel,QAdjustPoint);
	~Linear();

private:
	typedef struct point_param
	{
		point_param()
		{}
		point_param(Image::Pixel syi_)
			: syi(syi_)
		{}
		int     dxi;
		int syi,dyi;
		double /*sx,*/dx;
		double /*sy,*/dy;
	} point_param;
	typedef QMap<Image::Pixel,point_param> point_params;
	point_params m_point;

	Image::Pixel calc(Image::Pixel);
};

#endif // IMAGINABLE__PLUGINS__COLOUR_ADJUST__LINEAR__INCLUDED
