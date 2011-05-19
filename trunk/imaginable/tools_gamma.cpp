/*************
**
** Project:      Imaginable
** File info:    $Id$
** Author:       Copyright (C) 2011 Kuzma Shapran <Kuzma.Shapran@gmail.com>
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


#include <cmath>

#include "tools_gamma.hpp"


namespace imaginable {

double gamma(double x,double k)
{
	if(k==0.)
		return x;
	if(k<0.)
		return (exp(-k*x)-1.)/(exp(-k)-1.);
	return log(x*(exp(k)-1.)+1.)/k;
}

}
