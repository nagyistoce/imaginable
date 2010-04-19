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


point& point::to_polar(void)
{
        double x_=x;
        double y_=y;
        r=sqrt(x_*x_+y_*y_);
        f=atan2(y_,x_);
        return *this;
}

point& point::to_rect(void)
{
        double r_=r;
        double f_=f;
        x=r_*cos(f_);
        y=r_*sin(f_);
        return *this;
}
