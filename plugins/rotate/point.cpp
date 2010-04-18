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


#include "point.hpp"

#include <cmath>


point::point(void)
	: x(0.)
	, y(0.)
{
}

point::point(double X,double Y)
	: x(X)
	, y(Y)
{
}

point::~point()
{
}

point point::polar(void) const
{
	point ret;
	ret.r=sqrt(x*x+y*y);
	ret.f=atan2(y,x);
	return ret;
}

point point::rect(void) const
{
	point ret;
	ret.x=r*cos(f);
	ret.y=r*sin(f);
	return ret;
}
